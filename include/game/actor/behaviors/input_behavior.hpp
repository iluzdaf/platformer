#pragma once

#include "game/actor/actor_behavior.hpp"

class InputBehavior : public ActorBehavior
{
public:
    void setIntentions(const InputIntentions &intentions);
    InputIntentions decide(float deltaTime, const ActorBehaviorContext &context) override;

private:
    InputIntentions intentions;
};
