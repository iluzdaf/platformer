#include "rendering/screen_transition.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

ScreenTransition::ScreenTransition(const Shader &shader)
    : active(false),
      alpha(0.0f),
      duration(1.0f),
      timer(0.0f),
      fadeIn(true),
      shader(shader)
{
    initQuad();
}

void ScreenTransition::initQuad()
{
    float vertices[] = {
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, -1.0f,

        -1.0f, 1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ScreenTransition::start(float duration, bool in)
{
    active = true;
    duration = duration;
    timer = 0.0f;
    fadeIn = in;
    alpha = in ? 1.0f : 0.0f;
}

void ScreenTransition::update(float deltaTime)
{
    if (!active)
        return;

    timer += deltaTime;
    float t = std::min(timer / duration, 1.0f);
    alpha = fadeIn ? (1.0f - t) : t;

    if (t >= 1.0f)
    {
        active = false;
    }
}

void ScreenTransition::draw()
{
    if (!active)
        return;

    shader.use();
    shader.setFloat("uAlpha", alpha);
    shader.setVec4("uColor", glm::vec4(0, 0, 0, 1.0f));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

bool ScreenTransition::isActive() const
{
    return active;
}

ScreenTransition::~ScreenTransition()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}