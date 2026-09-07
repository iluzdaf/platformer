#include "actor/observing.hpp"
#include "actor/actor_contact_state.hpp"
#include "physics/aabb.hpp"
#include "physics/physics_body.hpp"
#include "tile_map/tile_map.hpp"

ActorContactState contactsForANewFrame(ActorContactState contacts)
{
    contacts.collisionAABBX = AABB();
    contacts.collisionAABBY = AABB();
    contacts.bumpedCeiling = false;
    return contacts;
}

ActorContactState contactsAfterStep(
    const ActorContactState &previous,
    const PhysicsBody &physicsBody,
    const TileMap &tileMap)
{
    ActorContactState contacts = previous;
    contacts.wasOnGround = previous.onGround;
    contacts.onGround = physicsBody.contactWithGround(tileMap);
    contacts.wasHitCeiling = previous.hitCeiling;
    contacts.hitCeiling = physicsBody.contactWithCeiling(tileMap);
    if (contacts.hitCeiling)
        contacts.bumpedCeiling = true;

    contacts.touchingRightWall = physicsBody.contactWithRightWall(tileMap);
    contacts.touchingLeftWall = physicsBody.contactWithLeftWall(tileMap);
    contacts.grippableLeftWall = physicsBody.gripOnLeftWall(tileMap);
    contacts.grippableRightWall = physicsBody.gripOnRightWall(tileMap);
    contacts.ledgeOnLeft =
        contacts.touchingLeftWall && !physicsBody.contactWithLeftWallAtHead(tileMap);
    contacts.ledgeOnRight =
        contacts.touchingRightWall && !physicsBody.contactWithRightWallAtHead(tileMap);

    if (contacts.grippableLeftWall)
        contacts.wasLastWallLeft = true;
    else if (contacts.grippableRightWall)
        contacts.wasLastWallLeft = false;

    contacts.collisionAABBX.expandToInclude(physicsBody.collisionAABBX());
    contacts.collisionAABBY.expandToInclude(physicsBody.collisionAABBY());
    return contacts;
}
