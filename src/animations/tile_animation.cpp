#include "animations/tile_animation.hpp"

TileAnimation::TileAnimation(const TileAnimationData &tileAnimationData)
    : frameAnimation(tileAnimationData.frameAnimationData)
{
}

void TileAnimation::update(float deltaTime)
{
    frameAnimation.update(deltaTime);
}

int TileAnimation::getCurrentFrame() const
{
    return frameAnimation.getCurrentFrame();
}