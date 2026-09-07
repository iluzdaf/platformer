#include "actor/actor_motion.hpp"
#include "actor/actor_motion_data.hpp"
#include "actor/hit.hpp"
#include "physics/physics_body.hpp"
#include "tile_map/tile_map.hpp"

ActorMotion::ActorMotion(const ActorMotionData &data) : abilitySystem(data)
{
}

void ActorMotion::applyMovement(float deltaTime, const InputIntentions &inputIntentions)
{
    abilitySystem.applyMovement(deltaTime, inputIntentions, observations, state);
    observations.hits.clear();
}

void ActorMotion::readContacts(const PhysicsBody &physicsBody, const TileMap &tileMap)
{
    observations.contacts.wasOnGround = observations.contacts.onGround;
    observations.contacts.onGround = physicsBody.contactWithGround(tileMap);
    observations.contacts.wasHitCeiling = observations.contacts.hitCeiling;
    observations.contacts.hitCeiling = physicsBody.contactWithCeiling(tileMap);
    if (observations.contacts.hitCeiling)
        observations.contacts.bumpedCeiling = true;
    observations.contacts.touchingRightWall = physicsBody.contactWithRightWall(tileMap);
    observations.contacts.touchingLeftWall = physicsBody.contactWithLeftWall(tileMap);
    observations.contacts.grippableLeftWall = physicsBody.gripOnLeftWall(tileMap);
    observations.contacts.grippableRightWall = physicsBody.gripOnRightWall(tileMap);
    observations.contacts.ledgeOnLeft =
        observations.contacts.touchingLeftWall && !physicsBody.contactWithLeftWallAtHead(tileMap);
    observations.contacts.ledgeOnRight =
        observations.contacts.touchingRightWall && !physicsBody.contactWithRightWallAtHead(tileMap);
    if (observations.contacts.grippableLeftWall)
        observations.contacts.wasLastWallLeft = true;
    else if (observations.contacts.grippableRightWall)
        observations.contacts.wasLastWallLeft = false;
    observations.contacts.collisionAABBX.expandToInclude(physicsBody.collisionAABBX());
    observations.contacts.collisionAABBY.expandToInclude(physicsBody.collisionAABBY());
}

void ActorMotion::readMotion(const PhysicsBody &physicsBody)
{
    state.previousVelocity = state.velocity;
    state.velocity = physicsBody.velocity();
}

void ActorMotion::pushedBy(const Hit &hit)
{
    observations.hits.push_back(hit);
}

void ActorMotion::beginFrame()
{
    observations.contacts.collisionAABBX = AABB();
    observations.contacts.collisionAABBY = AABB();
    observations.contacts.bumpedCeiling = false;
}

const ActorMotionState &ActorMotion::getState() const
{
    return state;
}

const Observed &ActorMotion::observed() const
{
    return observations;
}
