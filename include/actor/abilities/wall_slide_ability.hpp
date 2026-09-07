#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct Decided;

class WallSlideAbility : public Ability
{
public:
    explicit WallSlideAbility(const WallSlideAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &decided) override;

private:
    WallSlideAbilityData data;
};