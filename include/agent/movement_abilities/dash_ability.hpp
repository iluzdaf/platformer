#pragma once

#include "agent/movement_abilities/movement_ability.hpp"
#include "agent/movement_abilities/dash_ability_data.hpp"

struct InputIntentions;
struct AgentState;

class DashAbility : public MovementAbility
{
public:
    explicit DashAbility(const DashAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        AgentState &state) override;

private:
    DashAbilityData data;
};