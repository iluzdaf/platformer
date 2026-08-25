#pragma once

struct DirectionBuffer
{
    void press(float directionX);
    float getBufferedDirectionX() const;
    void consume();

    float bufferedDirectionX = 0.0f;
};
