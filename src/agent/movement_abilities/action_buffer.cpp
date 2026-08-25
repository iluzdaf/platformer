#include <algorithm>
#include <stdexcept>
#include "agent/movement_abilities/action_buffer.hpp"

ActionBuffer::ActionBuffer(float duration)
    : bufferDuration(duration)
{
    if (duration <= 0)
        throw std::runtime_error("duration must be greater than 0");
}

void ActionBuffer::press()
{
    bufferTimer = bufferDuration;
}

void ActionBuffer::update(float dt)
{
    bufferTimer = std::max(0.0f, bufferTimer - dt);
}

bool ActionBuffer::isBuffered() const
{
    return bufferTimer > 0.0f;
}

void ActionBuffer::consume()
{
    bufferTimer = 0.0f;
}
