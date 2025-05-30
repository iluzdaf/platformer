#pragma once
#include "animations/frame_animation_data.hpp"

struct SpriteAnimationData
{
    FrameAnimationData frameAnimationData;
    int frameWidth = 16;
    int frameHeight = 16;
    int textureWidth = 96;
};