#pragma once
#include "animations/frame_animation.hpp"
class TileAnimationData;

class TileAnimation
{
public:
    explicit TileAnimation(const TileAnimationData& tileAnimationData);
    void update(float deltaTime);
    int getCurrentFrame() const;

private:
    FrameAnimation frameAnimation;
};