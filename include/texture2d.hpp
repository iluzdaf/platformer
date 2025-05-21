#pragma once
#include <glad/glad.h>
#include <string>

class Texture2D
{
public:
    Texture2D(const std::string &filePath);
    ~Texture2D();
    void bind() const;
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    bool valid() const;

private:
    GLuint textureID;
    int width, height, channels;
};