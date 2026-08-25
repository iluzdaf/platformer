#pragma once

#include "animations/sprite_animation_data.hpp"

struct ActorAnimationData
{
    SpriteAnimationData idleSpriteAnimationData;
    SpriteAnimationData walkSpriteAnimationData;
    SpriteAnimationData dashSpriteAnimationData;
    SpriteAnimationData jumpSpriteAnimationData;
    SpriteAnimationData fallSpriteAnimationData;
    SpriteAnimationData wallSlideSpriteAnimationData;
};
