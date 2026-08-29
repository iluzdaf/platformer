#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"

struct InputIntentions;
struct ActorMotionState;

class WallHangAbility : public Ability
{
public:
    explicit WallHangAbility(const WallHangAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;
};