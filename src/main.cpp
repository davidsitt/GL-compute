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

    GLFWwindow *window = glfwCreateWindow(100, 100, "Framebuffer", NULL, NULL);
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
    FrameBuffer fbo;
    fbo.Create(width, height);
    fbo.Bind();
    glViewport(0, 0, width, height);

    shader.Use();
    shader.SetUniform("width", 120);
    shader.SetUniform("height", 64);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(window);

    fbo.UnBind();

    // std::vector<uint8_t> data;
    // fbo.Color_0().ToBuffer(data);
    cv::Mat output;
    fbo.Color_0().ToMat(output);

    cv::imshow("Output", output);
    cv::waitKey(0);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;

    /////////////////////////////////////////////

    // while (!glfwWindowShouldClose(window))
    // {
    //     glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    //     glClear(GL_COLOR_BUFFER_BIT);

    //     shader.Use();
    //     glBindVertexArray(VAO);
    //     glDrawArrays(GL_TRIANGLES, 0, 6);

    //     glfwSwapBuffers(window);
    //     glfwPollEvents();
    // }
    // glfwTerminate();
    // return 0;
}
