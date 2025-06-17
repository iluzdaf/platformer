#include <stdexcept>
#include "rendering/texture2d.hpp"
#include "stb_image.h"

Texture2D::Texture2D(const std::string &filePath)
{
    if (filePath.empty())
        throw std::invalid_argument("Texture2D filePath must not be empty");

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

std::pair<glm::vec2, glm::vec2> Texture2D::getUVRange(int frameIndex, int tileSize, bool flipY) const
{
    int tilesPerRow = width / tileSize;
    int tileX = frameIndex % tilesPerRow;
    int tileY = frameIndex / tilesPerRow;
    float uvSize = static_cast<float>(tileSize) / static_cast<float>(width);

    if (flipY)
        return {glm::vec2(tileX * uvSize, tileY * uvSize),
                glm::vec2((tileX + 1) * uvSize, (tileY + 1) * uvSize)};
    else
        return {glm::vec2(tileX * uvSize, (tileY + 1) * uvSize),
                glm::vec2((tileX + 1) * uvSize, tileY * uvSize)};
}