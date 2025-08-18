#pragma once

#include "agent/movement_abilities/movement_ability.hpp"
#include "agent/movement_abilities/gravity_ability_data.hpp"

struct InputIntentions;
struct AgentState;

class GravityAbility : public MovementAbility
{
public:
    explicit GravityAbility(const GravityAbilityData &data);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        AgentState &state) override;

private:
    GravityAbilityData data;
};