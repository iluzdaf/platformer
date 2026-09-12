#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/knockback_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct AbilityStates;

class KnockbackAbility : public Ability
{
public:
    explicit KnockbackAbility(const KnockbackAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        AbilityStates &states) override;

private:
    KnockbackAbilityData data;
};
