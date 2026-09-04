#pragma once
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
class Texture2D;
class Shader;

class SpriteRenderer
{
public:
    SpriteRenderer();
    ~SpriteRenderer();
    SpriteRenderer(const SpriteRenderer &) = delete;
    SpriteRenderer &operator=(const SpriteRenderer &) = delete;
    void draw(
        const Shader &shader,
        const Texture2D &texture,
        glm::mat4 projection,
        glm::vec2 position,
        glm::vec2 size,
        glm::vec2 uvStart = glm::vec2(0.0f, 0.0f),
        glm::vec2 uvEnd = glm::vec2(1.0f, 1.0f),
        bool flipX = false) const;

private:
    GLuint quadVertexArrayObject = 0;
};