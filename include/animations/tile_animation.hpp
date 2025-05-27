#pragma once
#include "animations/frame_animation.hpp"
#include "animations/tile_animation_data.hpp"

class TileAnimation
{
public:
    explicit TileAnimation(const TileAnimationData& tileAnimationData);
    void update(float deltaTime);
    int getCurrentFrame() const;

private:
    FrameAnimation frameAnimation;
};