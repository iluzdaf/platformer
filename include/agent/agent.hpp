#pragma once

#include "agent/agent_data.hpp"
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/movement_system.hpp"

class TileMap;
class PhysicsBody;
struct InputIntentions;

class Agent
{
public:
    explicit Agent(const AgentData &data);
    void applyMovement(float deltaTime, const InputIntentions &inputIntentions);
    void readContacts(const PhysicsBody &physicsBody, const TileMap &tileMap);
    void readMotion(const PhysicsBody &physicsBody);
    void resetCollisionAABB();
    const AgentState &getState() const;
    const MovementSystem &getMovementSystem() const;

private:
    AgentState state;
    MovementSystem movementSystem;
};
