#pragma once

#include "actor/abilities/ability.hpp"
#include "actor/abilities/dash_ability_data.hpp"

struct InputIntentions;
struct ActorMotionState;

class DashAbility : public Ability
{
public:
    explicit DashAbility(const DashAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) override;

private:
    DashAbilityData data;
};