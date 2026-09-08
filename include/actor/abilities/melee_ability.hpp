#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/melee_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct Decided;

class MeleeAbility : public Ability
{
public:
    explicit MeleeAbility(const MeleeAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &decided) override;

private:
    MeleeAbilityData data;
};
