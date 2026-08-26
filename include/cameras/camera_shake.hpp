#pragma once

#include <glm/gtc/matrix_transform.hpp>

class CameraShake
{
public:
    void start(float shakeDuration, float shakeMagnitude);
    glm::vec2 getOffset(float deltaTime);
    bool isActive() const;

private:
    float duration = 0.0f;
    float magnitude = 0.0f;
    float timeElapsed = 0.0f;
    bool active = false;
};
