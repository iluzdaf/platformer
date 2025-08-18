#pragma once

#include "agent/agent_data.hpp"
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/movement_system.hpp"
#include "physics/physics_body.hpp"

class TileMap;
struct InputIntentions;

class Agent
{
public:
    explicit Agent(const AgentData &data);
    void fixedUpdate(
        float deltaTime,
        const TileMap &tileMap,
        const InputIntentions &inputIntensions);
    void resetCollisionAABB();
    const AgentState &getState() const;
    const PhysicsBody &getPhysicsBody() const;
    const MovementSystem &getMovementSystem() const;
    void setPosition(const glm::vec2 &position);

private:
    AgentData data;
    AgentState state;
    MovementSystem movementSystem;
    PhysicsBody physicsBody;

    void updateState();
    void updatePhysicsState(const TileMap &tileMap);
};