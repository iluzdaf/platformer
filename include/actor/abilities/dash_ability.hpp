#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/dash_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct AbilityStates;

class DashAbility : public Ability
{
public:
    explicit DashAbility(const DashAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        AbilityStates &states) override;

private:
    DashAbilityData data;
};