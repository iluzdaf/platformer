#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "texture2d.hpp"
#include "shader.hpp"

class SpriteRenderer
{
public:
    SpriteRenderer(const Shader &shader);
    ~SpriteRenderer();
    void draw(
        const Texture2D &texture,
        glm::mat4 projection,
        glm::vec2 position,
        glm::vec2 size = glm::vec2(32.0f, 32.0f),
        float rotate = 0.0f);
    void drawWithUV(
        const Texture2D &texture,
        glm::mat4 projection,
        glm::vec2 position,
        glm::vec2 size = glm::vec2(32.0f, 32.0f),
        glm::vec2 uvStart = glm::vec2(0.0f, 0.0f),
        glm::vec2 uvEnd = glm::vec2(1.0f, 1.0f));

private:
    GLuint quadVertexArrayObject;
    Shader shader;
};