#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/climb_ability_data.hpp"

struct InputIntentions;
struct ActorMotionState;

class ClimbAbility : public Ability
{
public:
    explicit ClimbAbility(const ClimbAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;
};