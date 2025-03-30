#ifndef Shader_hpp
#define Shader_hpp

#include "GL.hpp"
#include <iostream>

// Vertex Shader
const char *vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;
    
    out vec2 TexCoord;
    
    void main()
    {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
    )";

// Fragment Shader
const char *fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    in vec2 TexCoord;

    uniform int width;
    uniform int height;
    
    void main()
    {   
        //FragColor = vec4(width / 255.0, height / 255.0, 0.0, 1.0);
        FragColor = vec4(TexCoord, 0.0, 1.0);
    }
    )";

class Shader
{
public:
    Shader() {}
    ~Shader()
    {
        std::cout << "[Shader] delete" << std::endl;
        glDetachShader(_program, _vertexShader);
        glDetachShader(_program, _fragmentShader);

        glDeleteShader(_vertexShader);
        glDeleteShader(_fragmentShader);
        glDeleteProgram(_program);
    }

    void Build()
    {
        std::cout << "[Shader] building" << std::endl;
        _vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
        _fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
        _program = glCreateProgram();
        glAttachShader(_program, _vertexShader);
        glAttachShader(_program, _fragmentShader);
        glLinkProgram(_program);
        std::cout << "[Shader] built : " << _program << std::endl;
    }

    void Use()
    {
        glUseProgram(_program);
    }

    void SetUniform(const std::string &name, int value)
    {
        glUniform1i(glGetUniformLocation(_program, name.c_str()), value);
    }

    void SetUniform(const std::string &name, float value)
    {
        glUniform1f(glGetUniformLocation(_program, name.c_str()), value);
    }

private:
    GLuint compileShader(GLenum type, const char *source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        return shader;
    }

    GLuint _program;
    GLuint _vertexShader;
    GLuint _fragmentShader;
};

#endif // Shader_hpp