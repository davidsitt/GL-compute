#ifndef Texture_hpp
#define Texture_hpp

#include "GL.hpp"
#include <vector>

class Texture
{
public:
    Texture()
    {
    }

    GLuint ID() const { return _ID; }

    void Create(int width, int height)
    {
        _width = width;
        _height = height;

        std::cout << "[Texture] Creating : [" << _width << " x " << _height << "]" << std::endl;
        glGenTextures(1, &_ID);

        glBindTexture(GL_TEXTURE_2D, _ID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        std::cout << "[Texture] Created : " << _ID << std::endl;
    }

    void Bind()
    {
        glBindTexture(GL_TEXTURE_2D, _ID);
    }

    void UnBind()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void GetData(std::vector<uint8_t> &data)
    {
        Bind();
        data = std::vector<uint8_t>(_width * _height * 4);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        UnBind();
    }

private:
    GLuint _ID;
    int _width;
    int _height;
};

#endif // Texture_hpp