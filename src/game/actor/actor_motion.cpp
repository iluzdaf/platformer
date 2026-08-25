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
    state.contacts.wasOnGround = state.contacts.onGround;
    state.contacts.onGround = physicsBody.contactWithGround(tileMap);
    state.contacts.wasHitCeiling = state.contacts.hitCeiling;
    state.contacts.hitCeiling = physicsBody.contactWithCeiling(tileMap);
    state.contacts.touchingRightWall = physicsBody.contactWithRightWall(tileMap);
    state.contacts.touchingLeftWall = physicsBody.contactWithLeftWall(tileMap);
    if (state.contacts.touchingLeftWall)
        state.contacts.wasLastWallLeft = true;
    else if (state.contacts.touchingRightWall)
        state.contacts.wasLastWallLeft = false;
    if (!physicsBody.getCollisionAABBX().isEmpty())
        state.contacts.collisionAABBX.expandToInclude(physicsBody.getCollisionAABBX());
    if (!physicsBody.getCollisionAABBY().isEmpty())
        state.contacts.collisionAABBY.expandToInclude(physicsBody.getCollisionAABBY());
}

void ActorMotion::readMotion(const PhysicsBody &physicsBody)
{
    state.previousVelocity = state.velocity;
    state.velocity = physicsBody.getVelocity();
}

void ActorMotion::resetCollisionAABB()
{
    state.contacts.collisionAABBX = AABB();
    state.contacts.collisionAABBY = AABB();
}

const ActorMotionState &ActorMotion::getState() const
{
    return state;
}

const AbilitySystem &ActorMotion::getAbilitySystem() const
{
    return abilitySystem;
}
