
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <omp.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Shader.hpp"
#include "ShaderCompute.hpp"
#include "FrameBuffer.hpp"
#include "Quad.hpp"

// Convenience utils for durations
typedef std::chrono::milliseconds ms;
inline std::chrono::milliseconds toMS(std::chrono::high_resolution_clock::duration d)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(d);
}

/**
 * @brief Create an invisible window to hold an OpenGL context.
 * @return GLFWwindow*
 */
GLFWwindow *CreateWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Create the window [1x1] and invisible
    GLFWwindow *window = glfwCreateWindow(1, 1, "Framebuffer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 0;
    }

    return window;
}

/**
 * @brief Apply the filter to an image using the CPU.
 *
 * @param input The image to filter.
 * @param output The filtered image.
 * @param useParallel Should the function use parallel processing.
 *
 * @remark To avoid handling the clamping, the border pixels are ignored.
 */
void FilterCPU(const cv::Mat &input, cv::Mat &output, bool useParallel)
{
    CV_Assert(input.channels() == 3); // Ensure RGB

    cv::Mat kernel = (cv::Mat_<float>(3, 3) << 1, 1, 1,
                      1, -8, 1,
                      1, 1, 1); // Sharpen kernel

    int rows = input.rows;
    int cols = input.cols;

    // Create the output (same size and format as the input)
    output = cv::Mat::zeros(rows, cols, CV_8UC3);

    // Get the default number of threads before modifying
    int defaultThreads = omp_get_max_threads();

    // No parallel => 1 thread only
    if (!useParallel)
        omp_set_num_threads(1); // Disable parallelism by using a single thread

#pragma omp parallel for collapse(2) // Parallelize both y and x loops
    for (int y = 1; y < rows - 1; ++y)
    {
        for (int x = 1; x < cols - 1; ++x)
        {
            for (int c = 0; c < 3; ++c)
            { // R, G, B channels
                float sum = 0.0f;
                for (int ky = -1; ky <= 1; ++ky)
                    for (int kx = -1; kx <= 1; ++kx)
                        sum += input.at<cv::Vec3b>(y + ky, x + kx)[c] * kernel.at<float>(ky + 1, kx + 1);
                output.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(sum);
            }
        }
    }

    // Restore the default threads
    omp_set_num_threads(defaultThreads);
}

/**
 * @brief Apply the filter to an image using the GPU (via OpenGL vertex and fragment shaders)
 *
 * @param input The image to filter.
 * @param output The filtered image.
 * @param window The window holding the GL context.
 */
void FilterShader(const cv::Mat &input, cv::Mat &output, GLFWwindow *window)
{
    int width = input.cols;
    int height = input.rows;

    // Build the shader
    Shader shader;
    shader.Build();

    // Build the Quad
    GLuint VAO;
    BuildQuad(VAO);

    // Build the input texture
    Texture inputTexture;
    inputTexture.Load(input);

    // Build and bind the FrameBuffer (same size as the input)
    FrameBuffer fbo;
    fbo.Create(width, height);
    fbo.Bind();
    glViewport(0, 0, width, height);

    // Set Shader input
    shader.Use();
    shader.SetUniform("width", static_cast<float>(width));
    shader.SetUniform("height", static_cast<float>(height));
    shader.SetTexture("inputTexture", inputTexture);

    // Draw
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Swap buffer and unbind
    glfwSwapBuffers(window);
    fbo.UnBind();

    // Get the output
    fbo.Color_0().ToMat(output);
}

/**
 * @brief Filter the input image using compute shader.
 *
 * @param input The image to filter.
 * @param output The filtered image.
 */
void FilterComputeShader(const cv::Mat &input, cv::Mat &output)
{
    const int width = input.cols;
    const int height = input.rows;

    // Add an alpha channel to the input
    cv::Mat inputRGBA;
    cv::cvtColor(input, inputRGBA, cv::COLOR_RGB2RGBA);

    // Input texture
    GLuint texIn;
    glGenTextures(1, &texIn);
    glBindTexture(GL_TEXTURE_2D, texIn);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, inputRGBA.data);

    // Output texture
    GLuint texOut;
    glGenTextures(1, &texOut);
    glBindTexture(GL_TEXTURE_2D, texOut);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

    // Compute Shader
    ShaderCompute shader;
    shader.Build();

    // Run
    shader.Use();
    glBindImageTexture(0, texIn, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(1, texOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glFinish();

    // Get the output
    cv::Mat resultRGBA(height, width, CV_8UC4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, resultRGBA.data);

    // Delete textures
    glDeleteTextures(1, &texIn);
    glDeleteTextures(1, &texOut);

    // Drop the alpha on the output
    cv::cvtColor(resultRGBA, output, cv::COLOR_RGBA2RGB);
}

/**
 * @brief Run a single fiter using both the CPU and the GPU and display the results.
 *
 * @param original The image to filter.
 * @param window The window holding the GL context.
 */
void RunSingle(const cv::Mat &original, GLFWwindow *window)
{
    cv::Mat outputCPU, outputShader, outputComputeShader;

    FilterCPU(original, outputCPU, false);
    FilterCPU(original, outputCPU, true);
    FilterShader(original, outputShader, window);
    FilterComputeShader(original, outputComputeShader);

    // Show
    cv::imshow("Output CPU", outputCPU);
    cv::imshow("Output GPU Shader", outputShader);
    cv::imshow("Output GPU Compute Shader", outputComputeShader);
    cv::waitKey(0);
}

/**
 * @brief Run a benchmark of the filtering run time.
 * Compare three different methods:
 *    # CPU no parallel processing
 *    # CPU with parallel processing
 *    # GPU
 *
 * Upscale the original image multiple times, then measure the run time of the three methods and print the results.
 *
 * @param original The image to filter.
 * @param window The window holding the GL context.
 */
void RunBench(const cv::Mat &original, GLFWwindow *window)
{
    // Storage for the bench
    std::vector<std::tuple<int, int, ms, ms, ms, ms>> timings;

    cv::Mat input, outputCPU, outputShader, outputComputeShader;
    std::vector<int> factors = {1, 2, 3, 4, 6, 8, 10};
    for (int factor : factors)
    {
        cv::resize(original, input, cv::Size(factor * original.cols, factor * original.rows));
        // Filter
        auto t0 = std::chrono::high_resolution_clock::now();
        FilterCPU(input, outputCPU, false);
        auto t1 = std::chrono::high_resolution_clock::now();
        FilterCPU(input, outputCPU, true);
        auto t2 = std::chrono::high_resolution_clock::now();
        FilterShader(input, outputShader, window);
        auto t3 = std::chrono::high_resolution_clock::now();
        FilterComputeShader(input, outputShader);
        auto t4 = std::chrono::high_resolution_clock::now();

        auto durationCPU = toMS(t1 - t0);
        auto durationCPU_MP = toMS(t2 - t1);
        auto durationShader = toMS(t3 - t2);
        auto durationComputeShader = toMS(t4 - t3);

        timings.push_back(std::make_tuple(factor, input.cols * input.rows, durationCPU, durationCPU_MP, durationShader, durationComputeShader));
    }

    std::cout << "Factor\tInput\tCPU\tCPU_MP\tShader\tCompute_Shader" << std::endl;
    for (auto &triplet : timings)
    {
        std::cout << std::get<0>(triplet) << "\t";
        std::cout << std::get<1>(triplet) << "\t";
        std::cout << std::get<2>(triplet).count() << "\t";
        std::cout << std::get<3>(triplet).count() << "\t";
        std::cout << std::get<4>(triplet).count() << "\t";
        std::cout << std::get<5>(triplet).count() << std::endl;
    }
}

int main()
{
    // Make the context current
    GLFWwindow *window = CreateWindow();
    glfwMakeContextCurrent(window);

    // initialise GLEW
    glewExperimental = GL_TRUE; // stops glew crashing on OSX :-/
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");

    // Load the input image and create the containers
    cv::Mat original = cv::imread("./res/montpellier.jpg");

    //********************************************* */
    RunSingle(original, window);
    // RunBench(original, window);
    //********************************************* */

    // Clean up
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
