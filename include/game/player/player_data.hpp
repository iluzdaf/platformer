#pragma once

#include "game/actor/actor_motion_data.hpp"
#include "game/actor/actor_animation_data.hpp"

struct PlayerData
{
    ActorMotionData motionData;
    ActorAnimationData animationData;

    float fallFromHeightThreshold = 600;
};
