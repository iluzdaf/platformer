#include "agent/agent.hpp"
#include "game/tile_map/tile_map.hpp"

Agent::Agent(const AgentData &data)
    : data(data),
      movementSystem(data),
      physicsBody(data.physicsBodyData)
{
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

const PhysicsBody &Agent::getPhysicsBody() const
{
    return physicsBody;
}

const MovementSystem &Agent::getMovementSystem() const
{
    return movementSystem;
}

void Agent::setPosition(const glm::vec2 &position)
{
    physicsBody.setPosition(position);
    state.position = position;
}

void Agent::updateState()
{
    state.position = physicsBody.getPosition();
    state.previousVelocity = state.velocity;
    state.velocity = physicsBody.getVelocity();
    state.colliderSize = physicsBody.getColliderSize();
    state.colliderOffset = physicsBody.getColliderOffset();
    state.size = data.size;
}

void Agent::updatePhysicsState(const TileMap &tileMap)
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

void Agent::fixedUpdate(
    float deltaTime,
    const TileMap &tileMap,
    const InputIntentions &inputIntensions)
{
    movementSystem.applyMovement(deltaTime, inputIntensions, state);

    physicsBody.setVelocity(state.targetVelocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    updatePhysicsState(tileMap);
    updateState();
}