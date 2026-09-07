#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/move_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct ActorMotionState;

class MoveAbility : public Ability
{
public:
    explicit MoveAbility(const MoveAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        ActorMotionState &state) override;

private:
    MoveAbilityData data;
};