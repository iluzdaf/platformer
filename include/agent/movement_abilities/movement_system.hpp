#pragma once

#include <vector>
#include <memory>
#include "agent/movement_abilities/movement_ability.hpp"

struct AgentData;
struct AgentState;
struct InputIntentions;

class MovementSystem
{
public:
    explicit MovementSystem(const AgentData &agentData);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        AgentState &state);

private:
    std::vector<std::unique_ptr<MovementAbility>> abilities;
};