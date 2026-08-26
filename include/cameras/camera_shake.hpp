#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct CameraShake
{
    float duration = 0.0f;
    float magnitude = 0.0f;
    float timeElapsed = 0.0f;
    bool active = false;

    void start(float shakeDuration, float shakeMagnitude);
    glm::vec2 getOffset(float deltaTime);
};
