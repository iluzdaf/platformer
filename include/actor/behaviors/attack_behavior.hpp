#pragma once

#include <string>
#include "actor/actor_behavior.hpp"
#include "actor/actor_behavior_context.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "input/input_intentions.hpp"

class AttackBehavior : public ActorBehavior
{
public:
    explicit AttackBehavior(const AttackBehaviorData &data);
    void reset() override;
    InputIntentions decide(float deltaTime, const ActorBehaviorContext &context) override;

private:
    std::string with;
    bool asked = false;
};
