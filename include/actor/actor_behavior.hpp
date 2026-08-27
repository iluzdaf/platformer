#pragma once

#include "actor/actor_behavior_context.hpp"
#include "input/input_intentions.hpp"

class ActorBehavior
{
public:
    virtual ~ActorBehavior() = default;
    virtual void reset();
    virtual InputIntentions decide(float deltaTime, const ActorBehaviorContext &context) = 0;
};
