#pragma once

#include "game/actor/actor_behavior_context.hpp"
#include "input/input_intentions.hpp"

class ActorBehavior
{
public:
    virtual ~ActorBehavior() = default;
    virtual void reset(const ActorBehaviorContext &context);
    virtual InputIntentions decide(float deltaTime, const ActorBehaviorContext &context) = 0;
};
