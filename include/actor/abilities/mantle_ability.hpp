#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/mantle_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct AbilityStates;

class MantleAbility : public Ability
{
public:
    explicit MantleAbility(const MantleAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        AbilityStates &states) override;

private:
    MantleAbilityData data;
};
