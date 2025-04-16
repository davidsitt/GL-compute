#ifndef Shader_hpp
#define Shader_hpp

#include <iostream>

#include "GL.hpp"
#include "Texture.hpp"

// Vertex Shader
const char *vertexShaderSource = R"(
    #version 430 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoords;
    
    out vec2 TexCoords;
    
    void main()
    {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoords = aTexCoords;
    }
    )";

// Fragment Shader
const char *fragmentShaderSource = R"(
    #version 430 core
    out vec3 FragColor;
    in vec2 TexCoords;

    uniform float width;
    uniform float height;

    uniform sampler2D inputTexture;
    
    void main()
    {
        vec2 dir = vec2(1.0 / width, 1.0 / height);  

        vec2 offsets[9] = vec2[](
            vec2(-dir.x,  dir.y), // top-left
            vec2( 0.0f,    dir.y), // top-center
            vec2( dir.x,  dir.y), // top-right
            vec2(-dir.x,  0.0f),   // center-left
            vec2( 0.0f,    0.0f),   // center-center
            vec2( dir.x,  0.0f),   // center-right
            vec2(-dir.x, -dir.y), // bottom-left
            vec2( 0.0f,   -dir.y), // bottom-center
            vec2( dir.x, -dir.y)  // bottom-right    
        );

        // Sharp
        /*float kernel[9] = float[](
            -1, -1, -1,
            -1,  9, -1,
            -1, -1, -1
        );*/

        // Edge (sum = 0 )
        float kernel[9] = float[](
            1,  1,  1,
            1, -8,  1,
            1,  1,  1
        );
    
        // Sample the input texture
        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
            sampleTex[i] = vec3(texture(inputTexture, TexCoords.st + offsets[i]));

        // Accumulate
        vec3 col = vec3(0.0);
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];
    
        FragColor = col;
    }
    )";

class Shader
{
public:
    Shader() : _program(0), _vertexShader(0), _fragmentShader(0) {}
    ~Shader()
    {
        if (_program != 0 && _vertexShader != 0)
            glDetachShader(_program, _vertexShader);

        if (_program != 0 && _fragmentShader != 0)
            glDetachShader(_program, _fragmentShader);

        if (_vertexShader != 0)
        {
            glDeleteShader(_vertexShader);
            _vertexShader = 0;
        }

        if (_fragmentShader != 0)
        {
            glDeleteShader(_fragmentShader);
            _fragmentShader = 0;
        }

        if (_program != 0)
        {
            std::cout << "[Shader] deleting : " << _program << std::endl;
            glDeleteProgram(_program);
            _program = 0;
        }
    }

    /**
     * Build vertex and fragment shader.
     * Create the program
     * Attach the shaders
     * Link
     * */
    void Build()
    {
        _vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
        _fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
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

    /**
     * @brief Set a uniform int.
     *
     * @param name The name of the uniform.
     * @param value The value of the uniform.
     */
    void SetUniform(const std::string &name, int value)
    {
        glUniform1i(glGetUniformLocation(_program, name.c_str()), value);
    }

    /**
     * @brief  Set a uniform float.
     *
     * @param name The name of the uniform.
     * @param value The value of the uniform.
     */
    void SetUniform(const std::string &name, float value)
    {
        glUniform1f(glGetUniformLocation(_program, name.c_str()), value);
    }

    /**
     * @brief Set a uniform texture
     *
     * @param name The name of the uniform.
     * @param texture The texture.
     * @todo Handle more texture unit.
     */
    void SetTexture(const std::string &name, const Texture &texture)
    {
        glActiveTexture(GL_TEXTURE0);
        texture.Bind();
        glUniform1i(glGetUniformLocation(_program, name.c_str()), 0);
    }

private:
    GLuint CompileShader(GLenum type, const char *source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        CheckCompileErrors(shader, "SHADER");
        return shader;
    }

    void CheckCompileErrors(GLuint shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << std::endl;
                std::cout << infoLog << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << std::endl;
                std::cout << infoLog << std::endl;
            }
        }
    }

private:
    GLuint _program;
    GLuint _vertexShader;
    GLuint _fragmentShader;
};

#endif // Shader_hpp