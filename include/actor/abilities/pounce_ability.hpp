#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/pounce_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct AbilityStates;

class PounceAbility : public Ability
{
public:
    explicit PounceAbility(const PounceAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        AbilityStates &states) override;

private:
    PounceAbilityData data;
};
