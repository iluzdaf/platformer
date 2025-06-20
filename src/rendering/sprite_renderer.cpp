#include "rendering/sprite_renderer.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/shader.hpp"

SpriteRenderer::SpriteRenderer()
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
    if (quadVertexArrayObject != 0)
    {
        glDeleteVertexArrays(1, &quadVertexArrayObject);
        quadVertexArrayObject = 0;
    }
}

void SpriteRenderer::draw(
    const Shader &shader,
    const Texture2D &texture,
    glm::mat4 projection,
    glm::vec2 position,
    glm::vec2 size,
    float rotate)
{
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

void SpriteRenderer::drawWithUV(
    const Shader &shader,
    const Texture2D &texture,
    glm::mat4 projection,
    glm::vec2 position,
    glm::vec2 size,
    glm::vec2 uvStart,
    glm::vec2 uvEnd,
    bool flipX)
{
    shader.use();
    shader.setMat4("projection", projection);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    if (flipX)
    {
        model = glm::translate(model, glm::vec3(size.x, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.0f, 1.0f));
    }
    model = glm::scale(model, glm::vec3(size, 1.0f));
    shader.setMat4("model", model);

    shader.setVec2("uvStart", uvStart);
    shader.setVec2("uvEnd", uvEnd);
    shader.setInt("image", 0);

    glActiveTexture(GL_TEXTURE0);
    texture.bind();

    glBindVertexArray(quadVertexArrayObject);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}