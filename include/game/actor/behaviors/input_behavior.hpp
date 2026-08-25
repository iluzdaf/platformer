#pragma once

#include "game/actor/actor_behavior.hpp"
#include "input/intention_source.hpp"

class InputBehavior : public ActorBehavior
{
public:
    explicit InputBehavior(const IntentionSource &intentionSource);
    InputIntentions decide(float deltaTime, const ActorBehaviorContext &context) override;

private:
    const IntentionSource &intentionSource;
};
