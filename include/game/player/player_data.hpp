#pragma once

#include "agent/agent_data.hpp"
#include "game/actor/actor_animation_data.hpp"

struct PlayerData
{
    AgentData agentData;
    ActorAnimationData animationData;

    float fallFromHeightThreshold = 600;
};
