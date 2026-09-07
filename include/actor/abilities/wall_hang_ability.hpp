#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"

struct InputIntentions;
struct Observed;
struct Decided;

class WallHangAbility : public Ability
{
public:
    explicit WallHangAbility(const WallHangAbilityData &data);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &decided) override;
};