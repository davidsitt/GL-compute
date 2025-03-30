#ifndef GL_hpp
#define GL_hpp

#include <GL/glew.h>

#include <iostream>

[[maybe_unused]]
static void PrintGLInfo()
{
    // print out some info about the graphics drivers
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
}

[[maybe_unused]]
static std::string GetErrorString(GLenum error)
{
    switch (error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return std::to_string(error);
    }
}

[[maybe_unused]]
static void CheckGLError(const std::string &step)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
        std::cout << step << " : " << GetErrorString(err) << std::endl;
}

#endif // GL_hpp