#pragma once

inline bool facingLeftAfter(bool facedLeft, float velocityX, bool knockedBack)
{
    if (knockedBack || velocityX == 0.0f)
        return facedLeft;

    return velocityX < 0.0f;
}
