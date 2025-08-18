#pragma once

#include "agent/movement_abilities/movement_ability.hpp"
#include "agent/movement_abilities/climb_move_ability_data.hpp"

struct InputIntentions;
struct AgentState;

class ClimbMoveAbility : public MovementAbility
{
public:
    explicit ClimbMoveAbility(const ClimbMoveAbilityData& data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        AgentState &state) override;

private:
    ClimbMoveAbilityData data;
};