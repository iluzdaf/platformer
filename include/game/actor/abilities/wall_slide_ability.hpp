#pragma once

#include "game/actor/abilities/ability.hpp"
#include "game/actor/abilities/wall_slide_ability_data.hpp"

struct InputIntentions;
struct ActorMotionState;

class WallSlideAbility : public Ability
{
public:
    explicit WallSlideAbility(const WallSlideAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;

private:
    WallSlideAbilityData data;
};