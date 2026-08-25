#pragma once

#include <optional>
#include "game/actor/actor_motion_data.hpp"
#include "game/actor/actor_animation_data.hpp"
#include "game/npc/behaviors/patrol_behavior_data.hpp"

struct NpcData
{
    ActorMotionData motionData;
    ActorAnimationData animationData;

    std::optional<PatrolBehaviorData> patrolBehaviorData;
};
