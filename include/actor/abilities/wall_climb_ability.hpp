#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct Decided;

class WallClimbAbility : public Ability
{
public:
    explicit WallClimbAbility(const WallClimbAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &decided) override;

private:
    WallClimbAbilityData data;
};