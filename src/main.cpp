#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "Shader.hpp"
#include "FrameBuffer.hpp"

// Quad vertices
float quadVertices[] = {
    // Positions   // TexCoords
    -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(1, 1, "Framebuffer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(window);

    // initialise GLEW
    glewExperimental = GL_TRUE; // stops glew crashing on OSX :-/
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");

    // Create VAO & VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    Shader shader;
    shader.Build();

    cv::Mat image = cv::imread("./res/montpellier.jpg");

    /////////////////////////////////////////////
    int width = image.cols;
    int height = image.rows;

    // Input data
    Texture input;
    input.Load(image);

    // FrameBuffer
    FrameBuffer fbo;
    fbo.Create(width, height);
    fbo.Bind();
    glViewport(0, 0, width, height);

    // Shader
    shader.Use();
    shader.SetUniform("width", static_cast<float>(width));
    shader.SetUniform("height", static_cast<float>(height));
    shader.SetTexture("inputTexture", input);

    // Draw
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glfwSwapBuffers(window);

    fbo.UnBind();

    cv::Mat output;
    fbo.Color_0().ToMat(output);

    cv::imshow("Output", output);
    cv::waitKey(0);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
