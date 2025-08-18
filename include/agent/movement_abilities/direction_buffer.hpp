#pragma once

#include <algorithm>
#include <stdexcept>

struct DirectionBuffer
{
    explicit DirectionBuffer(float duration = 0.1f)
        : bufferDuration(duration)
    {
        if (duration <= 0.0f)
            throw std::runtime_error("duration must be greater than 0");
    }

    void press(float directionX)
    {
        bufferTimer = bufferDuration;
        bufferedDirectionX = directionX;
    }

    void update(float dt)
    {
        bufferTimer = std::max(0.0f, bufferTimer - dt);
    }

    bool isBuffered() const
    {
        return bufferTimer > 0.0f;
    }

    float getBufferedDirectionX() const
    {
        return bufferedDirectionX;
    }

    void consume()
    {
        bufferTimer = 0.0f;
        bufferedDirectionX = 0.0f;
    }

    void reset()
    {
        bufferTimer = 0.0f;
        bufferedDirectionX = 0.0f;
    }

    const float bufferDuration;
    float bufferTimer = 0.0f;
    float bufferedDirectionX = 0.0f;
};