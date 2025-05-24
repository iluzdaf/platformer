#pragma once
#include "animations/frame_animation.hpp"

class TileAnimation
{
public:
    TileAnimation(std::vector<int> frames, float frameDuration);
    void update(float deltaTime);
    int getCurrentFrame() const;

private:
    FrameAnimation animation;
};