#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/bite_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct AbilityStates;

class BiteAbility : public Ability
{
public:
    explicit BiteAbility(const BiteAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        AbilityStates &states) override;

private:
    BiteAbilityData data;
};
