#pragma once
#include <glad/glad.h>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

std::pair<glm::vec2, glm::vec2> frameUvRangeIn(
    int textureWidth,
    int textureHeight,
    int frameIndex,
    int frameWidth,
    int frameHeight,
    bool flipY = true);

std::pair<glm::vec2, glm::vec2> uvRangeIn(
    int textureWidth,
    int textureHeight,
    int frameIndex,
    int tileSize,
    bool flipY = true);

class Texture2D
{
public:
    explicit Texture2D(const std::string &filePath);
    ~Texture2D();
    Texture2D(const Texture2D &) = delete;
    Texture2D &operator=(const Texture2D &) = delete;
    void bind() const;
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    GLuint getTextureID() const;
    std::pair<glm::vec2, glm::vec2> getUVRange(int frameIndex, int tileSize, bool flipY = true)
        const;

private:
    GLuint textureID = 0;
    int width = 0, height = 0, channels = 0;
};