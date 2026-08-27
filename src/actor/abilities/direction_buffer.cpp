#include "actor/abilities/direction_buffer.hpp"

void DirectionBuffer::press(float directionX)
{
    bufferedDirectionX = directionX;
}

float DirectionBuffer::getBufferedDirectionX() const
{
    return bufferedDirectionX;
}

void DirectionBuffer::consume()
{
    bufferedDirectionX = 0.0f;
}
