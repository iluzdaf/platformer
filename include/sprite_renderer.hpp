#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "texture2d.hpp"
#include "shader.hpp"

class SpriteRenderer
{
public:
    SpriteRenderer();
    ~SpriteRenderer();
    void draw(const Texture2D &texture,
              const Shader &shader,
              glm::mat4 projection,
              glm::vec2 position,
              glm::vec2 size = glm::vec2(32.0f, 32.0f),
              float rotate = 0.0f);

private:
    GLuint quadVertexArrayObject;
};