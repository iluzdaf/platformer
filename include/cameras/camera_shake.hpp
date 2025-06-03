#pragma once
#include <glm/gtc/matrix_transform.hpp>

struct CameraShake
{
    float duration = 0.0f;
    float magnitude = 0.0f;
    float timeElapsed = 0.0f;
    bool active = false;

    void start(float shakeDuration, float shakeMagnitude)
    {
        duration = shakeDuration;
        magnitude = shakeMagnitude;
        timeElapsed = 0.0f;
        active = true;
    }

    glm::vec2 getOffset(float deltaTime)
    {
        if (!active)
            return glm::vec2(0.0f);

        timeElapsed += deltaTime;
        if (timeElapsed >= duration)
        {
            active = false;
            return glm::vec2(0.0f);
        }

        float x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * magnitude;
        float y = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * magnitude;

        return glm::vec2(x, y);
    }
};