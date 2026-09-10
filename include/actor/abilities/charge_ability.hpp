#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/charge_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct Decided;

class ChargeAbility : public Ability
{
public:
    explicit ChargeAbility(const ChargeAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &decided) override;

private:
    ChargeAbilityData data;
};
