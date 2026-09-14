#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/lower_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct AbilityStates;

class LowerAbility : public Ability
{
public:
    explicit LowerAbility(const LowerAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        AbilityStates &states) override;

private:
    LowerAbilityData data;
};
