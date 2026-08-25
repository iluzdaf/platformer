#pragma once

class DirectionBuffer
{
public:
    void press(float directionX);
    float getBufferedDirectionX() const;
    void consume();

private:
    float bufferedDirectionX = 0.0f;
};
