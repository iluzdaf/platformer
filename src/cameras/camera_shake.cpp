#include <random>
#include "cameras/camera_shake.hpp"

CameraShake::CameraShake() : CameraShake(std::random_device{}())
{
}

CameraShake::CameraShake(std::mt19937::result_type seed) : generator(seed)
{
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

    float fading = magnitude * (1.0f - timeElapsed / duration);

    return glm::vec2(offsets(generator) * fading, offsets(generator) * fading);
}

bool CameraShake::isActive() const
{
    return active;
}
