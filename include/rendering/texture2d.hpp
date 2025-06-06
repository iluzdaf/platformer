#pragma once
#include <glad/glad.h>
#include <string>

class Texture2D
{
public:
    explicit Texture2D(const std::string &filePath);
    ~Texture2D();
    void bind() const;
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    bool valid() const;
    GLuint getTextureID() const;

private:
    GLuint textureID = 0;
    int width = 0, height = 0, channels = 0;
};