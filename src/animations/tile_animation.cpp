#include "animations/tile_animation.hpp"

TileAnimation::TileAnimation(std::vector<int> frames, float frameDuration)
    : animation(std::move(frames), frameDuration) {}

void TileAnimation::update(float deltaTime)
{
    animation.update(deltaTime);
}

int TileAnimation::getCurrentFrame() const
{
    return animation.getCurrentFrame();
}