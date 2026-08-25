#pragma once

#include "game/actor/abilities/ability.hpp"
#include "game/actor/abilities/move_ability_data.hpp"

struct InputIntentions;
struct ActorMotionState;

class MoveAbility : public Ability
{
public:
    explicit MoveAbility(const MoveAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;

private:
    MoveAbilityData data;
};