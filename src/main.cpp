
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Shader.hpp"
#include "FrameBuffer.hpp"
#include "Quad.hpp"

GLFWwindow *CreateWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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

void FilterCPU(const cv::Mat &input, cv::Mat &output)
{
    CV_Assert(input.channels() == 3); // Ensure RGB

    cv::Mat kernel = (cv::Mat_<float>(3, 3) << 1, 1, 1,
                      1, -8, 1,
                      1, 1, 1); // Sharpen kernel

    int rows = input.rows;
    int cols = input.cols;

    // Create the output (same size and format as the input)
    output = cv::Mat::zeros(rows, cols, CV_8UC3);

    for (int y = 1; y < rows - 1; ++y)
    {
        for (int x = 1; x < cols - 1; ++x)
        {
            for (int c = 0; c < 3; ++c)
            { // R, G, B channels
                float sum = 0.0f;
                for (int ky = -1; ky <= 1; ++ky)
                {
                    for (int kx = -1; kx <= 1; ++kx)
                    {
                        sum += input.at<cv::Vec3b>(y + ky, x + kx)[c] * kernel.at<float>(ky + 1, kx + 1);
                    }
                }
                output.at<cv::Vec3b>(y, x)[c] = cv::saturate_cast<uchar>(sum);
            }
        }
    }
}

void FilterGPU(const cv::Mat &input, cv::Mat &output, GLFWwindow *window)
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

int main()
{

    // Make the context current
    GLFWwindow *window = CreateWindow();
    glfwMakeContextCurrent(window);

    // initialise GLEW
    glewExperimental = GL_TRUE; // stops glew crashing on OSX :-/
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");

    // Load the input image
    cv::Mat input = cv::imread("./res/montpellier.jpg");
    cv::Mat outputCPU, outputGPU;

    // Filter
    auto start = std::chrono::high_resolution_clock::now();
    FilterCPU(input, outputCPU);
    auto middle = std::chrono::high_resolution_clock::now();
    FilterGPU(input, outputGPU, window);
    auto end = std::chrono::high_resolution_clock::now();

    auto durationCPU = std::chrono::duration_cast<std::chrono::milliseconds>(middle - start);
    auto durationGPU = std::chrono::duration_cast<std::chrono::milliseconds>(end - middle);

    std::cout << "CPU: " << durationCPU.count() << " ms" << std::endl;
    std::cout << "GPU: " << durationGPU.count() << " ms" << std::endl;

    // Show
    cv::imshow("Output CPU", outputCPU);
    // cv::imshow("Output GPU", outputGPU);
    cv::waitKey(0);

    // Clean up
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
