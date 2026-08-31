#include "actor/actor_motion.hpp"
#include "actor/actor_motion_data.hpp"
#include "physics/physics_body.hpp"
#include "tile_map/tile_map.hpp"

ActorMotion::ActorMotion(const ActorMotionData &data) : abilitySystem(data)
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
    if (state.contacts.hitCeiling)
        state.contacts.bumpedCeiling = true;
    state.contacts.touchingRightWall = physicsBody.contactWithRightWall(tileMap);
    state.contacts.touchingLeftWall = physicsBody.contactWithLeftWall(tileMap);
    state.contacts.grippableLeftWall = physicsBody.gripOnLeftWall(tileMap);
    state.contacts.grippableRightWall = physicsBody.gripOnRightWall(tileMap);
    state.contacts.ledgeOnLeft =
        state.contacts.touchingLeftWall && !physicsBody.contactWithLeftWallAtHead(tileMap);
    state.contacts.ledgeOnRight =
        state.contacts.touchingRightWall && !physicsBody.contactWithRightWallAtHead(tileMap);
    if (state.contacts.grippableLeftWall)
        state.contacts.wasLastWallLeft = true;
    else if (state.contacts.grippableRightWall)
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

void ActorMotion::beginFrame()
{
    state.contacts.collisionAABBX = AABB();
    state.contacts.collisionAABBY = AABB();
    state.contacts.bumpedCeiling = false;
}

const ActorMotionState &ActorMotion::getState() const
{
    return state;
}

const AbilitySystem &ActorMotion::getAbilitySystem() const
{
    return abilitySystem;
}
