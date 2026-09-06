#include <algorithm>
#include <functional>
#include "game/playback.hpp"

namespace
{
    constexpr float LongestCatchUp = 0.25f;
}

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
        fixedStep(timestepper.getMaxStep());
        endFrame(timestepper.getMaxStep());
        stepping = false;
    }
    else
    {
        float caughtUp = std::min(deltaTime, LongestCatchUp);
        timestepper.run(caughtUp, fixedStep);
        endFrame(caughtUp);
    }
}
