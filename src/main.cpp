#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

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
    cv::Mat inputImage = cv::imread("./res/montpellier.jpg");
    int width = inputImage.cols;
    int height = inputImage.rows;

    // Build the shader
    Shader shader;
    shader.Build();

    // Build the Quad
    GLuint VAO;
    BuildQuad(VAO);

    // Build the input texture
    Texture input;
    input.Load(inputImage);

    // Build and bind the FrameBuffer (same size as the input)
    FrameBuffer fbo;
    fbo.Create(width, height);
    fbo.Bind();
    glViewport(0, 0, width, height);

    // Set Shader input
    shader.Use();
    shader.SetUniform("width", static_cast<float>(width));
    shader.SetUniform("height", static_cast<float>(height));
    shader.SetTexture("inputTexture", input);

    // Draw
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Swap buffer and unbind
    glfwSwapBuffers(window);
    fbo.UnBind();

    // Get the output
    cv::Mat output;
    fbo.Color_0().ToMat(output);

    // Show
    cv::imshow("Output", output);
    cv::waitKey(0);

    // Clean up
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
