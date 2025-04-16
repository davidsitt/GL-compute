#ifndef ShaderCompute_hpp
#define ShaderCompute_hpp

#include <iostream>

#include "GL.hpp"

const char *shaderSource = R"(
    #version 430

    layout(local_size_x = 1, local_size_y = 1) in;

    layout(binding = 0, rgba8) uniform readonly image2D inputImage;
    layout(binding = 1, rgba8) uniform writeonly image2D outputImage;

    // Laplacian edge detection kernel
    const float kernel[9] = float[](
        1,  1,  1,
        1, -8,  1,
        1,  1,  1
    );

    void main() {
        ivec2 size = imageSize(inputImage);
        ivec2 pos = ivec2(gl_GlobalInvocationID.xy);

        if (pos.x >= size.x || pos.y >= size.y)
            return;

        vec3 sum = vec3(0.0);
        for (int ky = -1; ky <= 1; ky++) {
            for (int kx = -1; kx <= 1; kx++) {
                ivec2 offset = clamp(pos + ivec2(kx, ky), ivec2(0), size - 1);
                vec4 pixel = imageLoad(inputImage, offset);
                float weight = kernel[(ky + 1) * 3 + (kx + 1)];
                sum += pixel.rgb * weight;
            }
        }

        imageStore(outputImage, pos, vec4(sum, 1.0));
    }

 )";

class ShaderCompute
{
public:
    ShaderCompute() : _program(0), _shader(0) {}
    ~ShaderCompute()
    {
        if (_shader != 0)
        {
            glDeleteShader(_shader);
            _shader = 0;
        }

        if (_program != 0)
        {
            std::cout << "[Shader] deleting : " << _program << std::endl;
            glDeleteProgram(_program);
            _program = 0;
        }
    }

    void Use()
    {
        glUseProgram(_program);
    }

    void Build()
    {

        _shader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(_shader, 1, &shaderSource, nullptr);
        glCompileShader(_shader);

        GLint success;
        glGetShaderiv(_shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char log[512];
            glGetShaderInfoLog(_shader, 512, nullptr, log);
            std::cerr << "Shader compile error:\n"
                      << log << std::endl;
        }

        _program = glCreateProgram();
        glAttachShader(_program, _shader);
        glLinkProgram(_program);
    }

private:
    GLuint _program;
    GLuint _shader;
};

#endif // ShaderCompute_hpp