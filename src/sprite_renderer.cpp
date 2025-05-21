#include "sprite_renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cassert>

SpriteRenderer::SpriteRenderer() : quadVertexArrayObject(0)
{
    GLuint vertexBufferObject;
    float vertices[] = {
        0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &quadVertexArrayObject);
    glGenBuffers(1, &vertexBufferObject);
    glBindVertexArray(quadVertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &vertexBufferObject);
}

SpriteRenderer::~SpriteRenderer()
{
    glDeleteVertexArrays(1, &quadVertexArrayObject);
}

void SpriteRenderer::draw(
    const Texture2D &texture,
    const Shader &shader,
    glm::mat4 projection,
    glm::vec2 position,
    glm::vec2 size,
    float rotate)
{
    assert(texture.valid());
    assert(shader.valid());

    shader.use();
    shader.setInt("image", 0);
    shader.setMat4("projection", projection);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));
    shader.setMat4("model", model);

    glActiveTexture(GL_TEXTURE0);
    texture.bind();

    glBindVertexArray(quadVertexArrayObject);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}