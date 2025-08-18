#pragma once

struct InputIntentions;
struct AgentState;

class MovementAbility
{
public:
    virtual ~MovementAbility() = default;
    virtual void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        AgentState &state) = 0;
};