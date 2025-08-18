#pragma once

#include "agent/movement_abilities/movement_ability.hpp"
#include "agent/movement_abilities/climb_ability_data.hpp"

struct InputIntentions;
struct AgentState;

class ClimbAbility : public MovementAbility
{
public:
    explicit ClimbAbility(const ClimbAbilityData& data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        AgentState &state) override;
};