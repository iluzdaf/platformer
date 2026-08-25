#pragma once

#include "game/actor/abilities/ability.hpp"
#include "game/actor/abilities/climb_move_ability_data.hpp"

struct InputIntentions;
struct ActorMotionState;

class ClimbMoveAbility : public Ability
{
public:
    explicit ClimbMoveAbility(const ClimbMoveAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;

private:
    ClimbMoveAbilityData data;
};