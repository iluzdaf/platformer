#pragma once
#include "rendering/shader.hpp"

class ScreenTransition
{
public:
    ScreenTransition(const Shader &shader);
    ~ScreenTransition();
    void start(float duration, bool fadeIn = true);
    void update(float deltaTime);
    void render();

    bool isActive() const;

private:
    unsigned int VAO, VBO;
    Shader shader;

    float duration;
    float timer;
    float alpha;
    bool active;
    bool fadeIn;

    void initQuad();
};