#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/mantle_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct Decided;

class MantleAbility : public Ability
{
public:
    explicit MantleAbility(const MantleAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &state) override;

private:
    MantleAbilityData data;
};
