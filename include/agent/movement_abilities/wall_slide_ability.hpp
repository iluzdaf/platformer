#pragma once

#include "agent/movement_abilities/movement_ability.hpp"
#include "agent/movement_abilities/wall_slide_ability_data.hpp"

struct InputIntentions;
struct AgentState;

class WallSlideAbility : public MovementAbility
{
public:
    explicit WallSlideAbility(const WallSlideAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        AgentState &state) override;

private:
    WallSlideAbilityData data;
};