#include <algorithm>
#include <functional>
#include "game/playback.hpp"

void Playback::play()
{
    paused = false;
    stepping = false;
}

void Playback::pause()
{
    paused = true;
}

void Playback::step()
{
    paused = true;
    stepping = true;
}

bool Playback::isPaused() const
{
    return paused;
}

void Playback::advance(
    float deltaTime,
    const std::function<void()> &beginFrame,
    const std::function<void(float)> &fixedStep,
    const std::function<void(float)> &endFrame)
{
    if (paused && !stepping)
        return;

    beginFrame();

    if (stepping)
    {
        float once = std::min(deltaTime, timestepper.getMaxStep());
        fixedStep(once);
        endFrame(once);
        stepping = false;
    }
    else
    {
        timestepper.run(deltaTime, fixedStep);
        endFrame(deltaTime);
    }
}
