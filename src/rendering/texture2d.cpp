#include <stdexcept>
#include <string>
#include <utility>
#include "rendering/texture2d.hpp"
#include "stb_image.h"

Texture2D::Texture2D(const std::string &filePath)
{
    if (filePath.empty())
        throw std::runtime_error("Texture2D filePath must not be empty");

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!data)
        throw std::runtime_error("Failed to load Texture2D");

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

Texture2D::~Texture2D()
{
    if (textureID != 0)
    {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}

void Texture2D::bind() const
{
    glBindTexture(GL_TEXTURE_2D, textureID);
}

unsigned int Texture2D::getWidth() const
{
    return width;
}

unsigned int Texture2D::getHeight() const
{
    return height;
}

GLuint Texture2D::getTextureID() const
{
    return textureID;
}

std::pair<glm::vec2, glm::vec2> frameUvRangeIn(
    int textureWidth,
    int textureHeight,
    int frameIndex,
    int frameWidth,
    int frameHeight,
    bool flipY)
{
    int across = frameWidth > 0 ? textureWidth / frameWidth : 0;
    if (across <= 0 || frameHeight <= 0 || textureHeight <= 0)
        return {glm::vec2(0.0f), glm::vec2(1.0f)};

    int tileX = frameIndex % across;
    int tileY = frameIndex / across;
    float uvWidth = static_cast<float>(frameWidth) / static_cast<float>(textureWidth);
    float uvHeight = static_cast<float>(frameHeight) / static_cast<float>(textureHeight);

    if (flipY)
        return {
            glm::vec2(tileX * uvWidth, tileY * uvHeight),
            glm::vec2((tileX + 1) * uvWidth, (tileY + 1) * uvHeight)};

    return {
        glm::vec2(tileX * uvWidth, (tileY + 1) * uvHeight),
        glm::vec2((tileX + 1) * uvWidth, tileY * uvHeight)};
}

std::pair<glm::vec2, glm::vec2> uvRangeIn(
    int textureWidth,
    int textureHeight,
    int frameIndex,
    int tileSize,
    bool flipY)
{
    return frameUvRangeIn(textureWidth, textureHeight, frameIndex, tileSize, tileSize, flipY);
}

std::pair<glm::vec2, glm::vec2> Texture2D::getUVRange(int frameIndex, int tileSize, bool flipY)
    const
{
    return uvRangeIn(width, height, frameIndex, tileSize, flipY);
}