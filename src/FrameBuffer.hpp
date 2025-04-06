#ifndef FrameBuffer_hpp
#define FrameBuffer_hpp

#include "GL.hpp"
#include "Texture.hpp"

class FrameBuffer
{
public:
    FrameBuffer() : _ID(0) {}
    ~FrameBuffer()
    {
        if (_ID != 0)
        {
            std::cout << "[FrameBuffer] Deleting : " << _ID << std::endl;
            glDeleteFramebuffers(1, &_ID);
            _ID = 0;
        }
    }

    Texture &Color_0() { return _color_0; }

    void Create(int width, int height)
    {
        // Create and bind
        glGenFramebuffers(1, &_ID);
        glBindFramebuffer(GL_FRAMEBUFFER, _ID);

        // Create the texture
        _color_0.Create(width, height);
        _color_0.Bind();

        // Attach the texture
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _color_0.ID(), 0);

        // Check status
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Incomplete FrameBuffer");

        // Unbind
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        std::cout << "[FrameBuffer] Created : " << _ID << std::endl;
    }

    void Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _ID);
    }

    void UnBind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

private:
    GLuint _ID;

    Texture _color_0;
};

#endif // FrameBuffer_hpp