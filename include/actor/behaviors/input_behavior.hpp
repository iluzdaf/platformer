#pragma once

#include "actor/actor_behavior.hpp"
#include "input/intention_source.hpp"
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"

class InputBehavior : public ActorBehavior
{
public:
    explicit InputBehavior(const IntentionSource &intentionSource);
    InputIntentions decide(float deltaTime, const ActorFacts &context) override;

private:
    const IntentionSource &intentionSource;
};
