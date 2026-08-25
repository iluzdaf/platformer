#pragma once

#include <optional>
#include "animations/sprite_animation_data.hpp"

struct ActorAnimationData
{
    SpriteAnimationData idleSpriteAnimationData;
    std::optional<SpriteAnimationData> walkSpriteAnimationData;
    std::optional<SpriteAnimationData> dashSpriteAnimationData;
    std::optional<SpriteAnimationData> jumpSpriteAnimationData;
    std::optional<SpriteAnimationData> fallSpriteAnimationData;
    std::optional<SpriteAnimationData> wallSlideSpriteAnimationData;
};
