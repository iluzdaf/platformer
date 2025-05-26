#include "animations/tile_animation.hpp"

TileAnimation::TileAnimation(const TileAnimationData &tileAnimationData)
    : animation(std::move(tileAnimationData.frames), tileAnimationData.frameDuration)
{
}

void TileAnimation::update(float deltaTime)
{
    animation.update(deltaTime);
}

int TileAnimation::getCurrentFrame() const
{
    return animation.getCurrentFrame();
}