#pragma once

#include <random>
#include <glm/gtc/matrix_transform.hpp>

class CameraShake
{
public:
    CameraShake();
    explicit CameraShake(std::mt19937::result_type seed);
    void start(float shakeDuration, float shakeMagnitude);
    glm::vec2 getOffset(float deltaTime);
    bool isActive() const;

private:
    std::mt19937 generator;
    std::uniform_real_distribution<float> offsets{-1.0f, 1.0f};

    float duration = 0.0f;
    float magnitude = 0.0f;
    float timeElapsed = 0.0f;
    bool active = false;
};
