#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/gravity_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct Decided;

class GravityAbility : public Ability
{
public:
    explicit GravityAbility(const GravityAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &decided) override;

private:
    GravityAbilityData data;
};