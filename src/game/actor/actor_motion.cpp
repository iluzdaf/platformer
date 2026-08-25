#include "game/actor/actor_motion.hpp"
#include "physics/physics_body.hpp"
#include "game/tile_map/tile_map.hpp"

ActorMotion::ActorMotion(const ActorMotionData &data)
    : abilitySystem(data)
{
}

void ActorMotion::applyMovement(float deltaTime, const InputIntentions &inputIntentions)
{
    abilitySystem.applyMovement(deltaTime, inputIntentions, state);
}

void ActorMotion::readContacts(const PhysicsBody &physicsBody, const TileMap &tileMap)
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

void ActorMotion::readMotion(const PhysicsBody &physicsBody)
{
    state.previousVelocity = state.velocity;
    state.velocity = physicsBody.getVelocity();
}

void ActorMotion::resetCollisionAABB()
{
    state.collisionAABBX = AABB();
    state.collisionAABBY = AABB();
}

const ActorMotionState &ActorMotion::getState() const
{
    return state;
}

const AbilitySystem &ActorMotion::getAbilitySystem() const
{
    return abilitySystem;
}
