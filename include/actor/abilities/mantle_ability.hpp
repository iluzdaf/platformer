#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/mantle_ability_data.hpp"

struct InputIntentions;
struct ActorMotionState;

class MantleAbility : public Ability
{
public:
    explicit MantleAbility(const MantleAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;

private:
    MantleAbilityData data;
};
