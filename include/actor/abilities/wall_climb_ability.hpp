#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"

struct InputIntentions;
struct ActorMotionState;

class WallClimbAbility : public Ability
{
public:
    explicit WallClimbAbility(const WallClimbAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;

private:
    WallClimbAbilityData data;
};