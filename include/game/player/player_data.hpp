#pragma once

#include "agent/agent_data.hpp"
#include "animations/sprite_animation_data.hpp"

struct PlayerData
{
    AgentData agentData;

    SpriteAnimationData idleSpriteAnimationData;
    SpriteAnimationData walkSpriteAnimationData;
    SpriteAnimationData dashSpriteAnimationData;
    SpriteAnimationData jumpSpriteAnimationData;
    SpriteAnimationData fallSpriteAnimationData;
    SpriteAnimationData wallSlideSpriteAnimationData;

    float fallFromHeightThreshold = 600;
};