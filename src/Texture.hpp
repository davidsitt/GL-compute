#ifndef Texture_hpp
#define Texture_hpp

#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include "GL.hpp"

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

        glGenTextures(1, &_ID);

        glBindTexture(GL_TEXTURE_2D, _ID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        std::cout << "[Texture] Created : " << _ID << " [" << _width << " x " << _height << "]" << std::endl;
    }

    void Load(const cv::Mat &image)
    {
        _width = image.cols;
        _height = image.rows;

        glGenTextures(1, &_ID);

        glBindTexture(GL_TEXTURE_2D, _ID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        std::cout << "[Texture] Loaded from image : " << _ID << " [" << _width << " x " << _height << "]" << std::endl;
    }

    void Bind() const
    {
        glBindTexture(GL_TEXTURE_2D, _ID);
    }

    void UnBind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void ToBuffer(std::vector<uint8_t> &data)
    {
        data = std::vector<uint8_t>(_width * _height * 4);

        Bind();
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
        UnBind();
    }

    void ToMat(cv::Mat &mat)
    {
        mat = cv::Mat(_height, _width, CV_8UC3);
        Bind();
        glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, mat.data);
        UnBind();
    }

    GLuint _ID;
    int _width;
    int _height;
};

#endif // Texture_hpp