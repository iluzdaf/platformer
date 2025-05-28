#include "rendering/texture2d.hpp"
#include "stb_image.h"
#include <cassert>

Texture2D::Texture2D(const std::string &filePath)
{
    assert(!filePath.empty());

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!data)
    {
        throw std::runtime_error("Failed to load Texture2D");
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

Texture2D::~Texture2D()
{
    if (valid())
    {
        glDeleteTextures(1, &textureID);
    }
}

void Texture2D::bind() const
{
    assert(valid());

    glBindTexture(GL_TEXTURE_2D, textureID);
}

unsigned int Texture2D::getWidth() const { return width; }

unsigned int Texture2D::getHeight() const { return height; }

bool Texture2D::valid() const { return textureID != 0; }