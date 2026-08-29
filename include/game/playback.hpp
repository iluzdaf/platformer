#pragma once

#include <functional>
#include "timing/fixed_time_step.hpp"

class Playback
{
public:
    void play();
    void pause();
    void step();
    bool isPaused() const;

    void advance(
        float deltaTime,
        const std::function<void()> &beginFrame,
        const std::function<void(float)> &fixedStep,
        const std::function<void(float)> &endFrame);

private:
    FixedTimeStep timestepper;
    bool paused = false, stepping = false;
};
