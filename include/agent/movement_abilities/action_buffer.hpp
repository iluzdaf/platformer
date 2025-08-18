#pragma once

#include <algorithm>
#include <stdexcept>

struct ActionBuffer
{
    ActionBuffer(float duration = 0.1f) : bufferDuration(duration)
    {
        if (duration <= 0)
            throw std::runtime_error("duration must be greater than 0");
    }

    void press() { bufferTimer = bufferDuration; }
    void update(float dt) { bufferTimer = std::max(0.0f, bufferTimer - dt); }
    bool isBuffered() const { return bufferTimer > 0.0f; }
    void consume() { bufferTimer = 0.0f; }

    float bufferDuration = 0.1f;
    float bufferTimer = 0.0f;
};