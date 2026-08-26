#include <cstdlib>
#include "cameras/camera_shake.hpp"

namespace
{
    float randomOffset(float magnitude)
    {
        float unitRandom = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return (unitRandom - 0.5f) * 2.0f * magnitude;
    }
}

void CameraShake::start(float shakeDuration, float shakeMagnitude)
{
    duration = shakeDuration;
    magnitude = shakeMagnitude;
    timeElapsed = 0.0f;
    active = true;
}

glm::vec2 CameraShake::getOffset(float deltaTime)
{
    if (!active)
        return glm::vec2(0.0f);

    timeElapsed += deltaTime;
    if (timeElapsed >= duration)
    {
        active = false;
        return glm::vec2(0.0f);
    }

    return glm::vec2(randomOffset(magnitude), randomOffset(magnitude));
}
