#include <functional>
#include "physics/physics_body.hpp"
#include "game/tile_map/tile_map.hpp"

void PhysicsBody::setPosition(glm::vec2 newPosition)
{
    position = newPosition;
}

glm::vec2 PhysicsBody::getPosition() const
{
    return position;
}

void PhysicsBody::setVelocity(glm::vec2 newVelocity)
{
    velocity = newVelocity;
}

glm::vec2 PhysicsBody::getVelocity() const
{
    return velocity;
}

void PhysicsBody::setColliderSize(glm::vec2 size)
{
    this->colliderSize = size;
}

glm::vec2 PhysicsBody::getColliderSize() const
{
    return colliderSize;
}

void PhysicsBody::setGravity(float newGravity)
{
    gravity = newGravity;
}

float PhysicsBody::getGravity() const
{
    return gravity;
}

AABB PhysicsBody::getAABB() const
{
    return AABB(position + colliderOffset, colliderSize);
}

void PhysicsBody::resolveCollisionAgainstTile(
    const AABB &proposedAABB,
    const AABB &tileAABB,
    glm::vec2 axisMask,
    float &velocityComponent,
    glm::vec2 &positionWithOffset,
    AABB &collisionAABB)
{
    glm::vec2 delta = proposedAABB.center() - tileAABB.center();
    glm::vec2 overlap = (tileAABB.size + proposedAABB.size) * 0.5f - glm::abs(delta);

    const float epsilon = 0.001f;
    if (overlap.x < epsilon || overlap.y < epsilon || glm::dot(delta, delta) < epsilon * epsilon)
        return;

    if (axisMask.x == 1.0f && overlap.x > epsilon)
    {
        if (velocityComponent < 0 && delta.x > 0)
            positionWithOffset.x += overlap.x;
        else if (velocityComponent > 0 && delta.x < 0)
            positionWithOffset.x -= overlap.x;
    }
    else if (axisMask.y == 1.0f && overlap.y > epsilon)
    {
        if (velocityComponent < 0 && delta.y > 0)
            positionWithOffset.y += overlap.y;
        else if (velocityComponent > 0 && delta.y < 0)
            positionWithOffset.y -= overlap.y;
    }

    velocityComponent = 0.0f;
    collisionAABB.expandToInclude(tileAABB);
}

void PhysicsBody::resolveHorizontalCollision(const TileMap &tileMap)
{
    collisionAABBX = AABB();
    glm::vec2 nextPositionWithOffset = nextPosition + colliderOffset;

    glm::vec2 reducedColliderSize = colliderSize;
    reducedColliderSize.y *= 0.5f;

    glm::vec2 positionOffset(0.0f);
    positionOffset.y = (colliderSize.y - reducedColliderSize.y) * 0.5f;

    glm::vec2 probePosition = nextPositionWithOffset + positionOffset;
    AABB proposedAABB(probePosition, reducedColliderSize);

    tileMap.probeSolidTiles(proposedAABB, [&](const AABB &tileAABB)
                            {
        resolveCollisionAgainstTile(proposedAABB, tileAABB, {1.0f, 0.0f}, nextVelocity.x, nextPositionWithOffset, collisionAABBX);
        return false; });

    nextPosition = nextPositionWithOffset - colliderOffset;
}

void PhysicsBody::resolveVerticalCollision(const TileMap &tileMap)
{
    collisionAABBY = AABB();
    glm::vec2 nextPositionWithOffset = nextPosition + colliderOffset;

    glm::vec2 reducedColliderSize = colliderSize;
    reducedColliderSize.x *= 0.5f;

    glm::vec2 positionOffset(0.0f);
    positionOffset.x = (colliderSize.x - reducedColliderSize.x) * 0.5f;

    glm::vec2 probePosition = nextPositionWithOffset + positionOffset;
    AABB proposedAABB(probePosition, reducedColliderSize);

    tileMap.probeSolidTiles(proposedAABB, [&](const AABB &tileAABB)
                            {
        resolveCollisionAgainstTile(proposedAABB, tileAABB, {0.0f, 1.0f}, nextVelocity.y, nextPositionWithOffset, collisionAABBY);
        return false; });

    nextPosition = nextPositionWithOffset - colliderOffset;
}

void PhysicsBody::clampToTileMapBounds(const TileMap &tileMap)
{
    const int mapWidth = tileMap.getWorldWidth();
    const int mapHeight = tileMap.getWorldHeight();

    glm::vec2 clampedPosition = nextPosition + colliderOffset;
    glm::vec2 clampedVelocity = nextVelocity;
    bool clamped = false;

    AABB playerAABB(clampedPosition, colliderSize);

    if (playerAABB.left() < 0.0f)
    {
        clampedPosition.x = 0.0f;
        clampedVelocity.x = 0.0f;
        clamped = true;
    }
    else if (playerAABB.right() > mapWidth)
    {
        clampedPosition.x = mapWidth - colliderSize.x;
        clampedVelocity.x = 0.0f;
        clamped = true;
    }

    if (playerAABB.top() < 0.0f)
    {
        clampedPosition.y = 0.0f;
        clampedVelocity.y = 0.0f;
        clamped = true;
    }
    else if (playerAABB.bottom() > mapHeight)
    {
        clampedPosition.y = mapHeight - colliderSize.y;
        clampedVelocity.y = 0.0f;
        clamped = true;
    }

    if (clamped)
    {
        nextPosition = clampedPosition - colliderOffset;
        nextVelocity = clampedVelocity;
    }
}

bool PhysicsBody::contactWithLeftWall(const TileMap &tileMap)
{
    glm::vec2 probeSize = colliderSize;
    probeSize.y *= 0.5f;
    glm::vec2 probePosition = position + colliderOffset + glm::vec2(-0.1f, 0.0f);
    probePosition.y += (colliderSize.y - probeSize.y) * 0.5f;
    AABB probeAABB(probePosition, probeSize);
    return tileMap.probeSolidTiles(probeAABB, [](const AABB &)
                                   { return true; });
}

bool PhysicsBody::contactWithRightWall(const TileMap &tileMap)
{
    glm::vec2 probeSize = colliderSize;
    probeSize.y *= 0.5f;
    glm::vec2 probePosition = position + colliderOffset + glm::vec2(0.1f, 0.0f);
    probePosition.y += (colliderSize.y - probeSize.y) * 0.5f;
    AABB probeAABB(probePosition, probeSize);
    return tileMap.probeSolidTiles(probeAABB, [](const AABB &)
                                   { return true; });
}

bool PhysicsBody::contactWithGround(const TileMap &tileMap)
{
    glm::vec2 probeSize = colliderSize;
    probeSize.x *= 0.5f;
    glm::vec2 probePosition = position + colliderOffset + glm::vec2(0.0f, 0.1f);
    probePosition.x += (colliderSize.x - probeSize.x) * 0.5f;
    AABB probeAABB(probePosition, probeSize);
    return tileMap.probeSolidTiles(probeAABB, [](const AABB &)
                                   { return true; }) ||
           AABB(position + colliderOffset, colliderSize).bottom() >= tileMap.getWorldHeight();
}

bool PhysicsBody::contactWithCeiling(const TileMap &tileMap) const
{
    glm::vec2 probeSize = colliderSize;
    probeSize.x *= 0.5f;
    glm::vec2 probePosition = position + colliderOffset + glm::vec2(0.0f, -0.1f);
    probePosition.x += (colliderSize.x - probeSize.x) * 0.5f;
    AABB probeAABB(probePosition, probeSize);
    return tileMap.probeSolidTiles(probeAABB, [](const AABB &)
                                   { return true; });
}

void PhysicsBody::applyGravity(float deltaTime)
{
    velocity.y += gravity * deltaTime;
}

void PhysicsBody::stepPhysics(float deltaTime, const TileMap &tileMap)
{
    nextPosition = position;
    nextVelocity = velocity;

    nextPosition += glm::vec2(nextVelocity.x, 0) * deltaTime;
    resolveHorizontalCollision(tileMap);

    nextPosition += glm::vec2(0, nextVelocity.y) * deltaTime;
    resolveVerticalCollision(tileMap);

    clampToTileMapBounds(tileMap);

    position = nextPosition;
    velocity = nextVelocity;
}

void PhysicsBody::setColliderOffset(glm::vec2 offset)
{
    colliderOffset = offset;
}

glm::vec2 PhysicsBody::getColliderOffset() const
{
    return colliderOffset;
}

AABB PhysicsBody::getCollisionAABBX() const
{
    return collisionAABBX;
}

AABB PhysicsBody::getCollisionAABBY() const
{
    return collisionAABBY;
}

void PhysicsBody::reset()
{
    position = glm::vec2(0, 0);
    nextPosition = glm::vec2(0, 0);
    velocity = glm::vec2(0);
    nextVelocity = glm::vec2(0);
    collisionAABBX = AABB();
    collisionAABBY = AABB();
}