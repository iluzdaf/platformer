#pragma once

#include "agent/movement_abilities/movement_ability.hpp"
#include "agent/movement_abilities/move_ability_data.hpp"

struct InputIntentions;
struct AgentState;

class MoveAbility : public MovementAbility
{
public:
    explicit MoveAbility(const MoveAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        AgentState &state) override;

private:
    MoveAbilityData data;
};