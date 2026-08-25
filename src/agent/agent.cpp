#include "agent/agent.hpp"
#include "physics/physics_body.hpp"
#include "game/tile_map/tile_map.hpp"

Agent::Agent(const AgentData &data)
    : data(data),
      movementSystem(data)
{
}

void Agent::applyMovement(float deltaTime, const InputIntentions &inputIntentions)
{
    movementSystem.applyMovement(deltaTime, inputIntentions, state);
}

void Agent::readContacts(const PhysicsBody &physicsBody, const TileMap &tileMap)
{
    state.wasOnGround = state.onGround;
    state.onGround = physicsBody.contactWithGround(tileMap);
    state.wasHitCeiling = state.hitCeiling;
    state.hitCeiling = physicsBody.contactWithCeiling(tileMap);
    state.touchingRightWall = physicsBody.contactWithRightWall(tileMap);
    state.touchingLeftWall = physicsBody.contactWithLeftWall(tileMap);
    if (state.touchingLeftWall)
        state.wasLastWallLeft = true;
    else if (state.touchingRightWall)
        state.wasLastWallLeft = false;
    if (!physicsBody.getCollisionAABBX().isEmpty())
        state.collisionAABBX.expandToInclude(physicsBody.getCollisionAABBX());
    if (!physicsBody.getCollisionAABBY().isEmpty())
        state.collisionAABBY.expandToInclude(physicsBody.getCollisionAABBY());
}

void Agent::readMotion(const PhysicsBody &physicsBody)
{
    state.position = physicsBody.getPosition();
    state.previousVelocity = state.velocity;
    state.velocity = physicsBody.getVelocity();
    state.colliderSize = physicsBody.getColliderSize();
    state.colliderOffset = physicsBody.getColliderOffset();
    state.size = data.size;
}

void Agent::resetCollisionAABB()
{
    state.collisionAABBX = AABB();
    state.collisionAABBY = AABB();
}

const AgentState &Agent::getState() const
{
    return state;
}

const MovementSystem &Agent::getMovementSystem() const
{
    return movementSystem;
}

void Agent::setPosition(const glm::vec2 &position)
{
    state.position = position;
}
