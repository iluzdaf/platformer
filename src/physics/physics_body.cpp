#include <functional>
#include "physics/physics_body.hpp"
#include "tile_map/tile_map.hpp"

PhysicsBody::PhysicsBody(const PhysicsBodyData &data)
    : data(data)
{
}

namespace
{
    constexpr float ContactProbeDepth = 0.1f;
}

void PhysicsBody::setPosition(const glm::vec2 &newPosition)
{
    position = newPosition;
}

const glm::vec2 &PhysicsBody::getPosition() const
{
    return position;
}

void PhysicsBody::setVelocity(const glm::vec2 &newVelocity)
{
    velocity = newVelocity;
}

const glm::vec2 &PhysicsBody::getVelocity() const
{
    return velocity;
}

const glm::vec2 &PhysicsBody::getColliderSize() const
{
    return data.colliderSize;
}

AABB PhysicsBody::getAABB() const
{
    return AABB(position + getColliderOffset(), getColliderSize());
}

glm::vec2 PhysicsBody::getBottomCenterOffset() const
{
    return getColliderOffset() + glm::vec2(getColliderSize().x * 0.5f, getColliderSize().y);
}

void PhysicsBody::resolveCollisionAgainstTile(
    const AABB &proposedAABB,
    const AABB &tileAABB,
    const glm::vec2 &axisMask,
    float &velocityComponent,
    glm::vec2 &positionWithOffset,
    AABB &collisionAABB)
{
    glm::vec2 delta = proposedAABB.center() - tileAABB.center();
    glm::vec2 overlap = (tileAABB.size + proposedAABB.size) * 0.5f - glm::abs(delta);

    const float epsilon = 0.001f;
    if (overlap.x < epsilon || overlap.y < epsilon || glm::dot(delta, delta) < epsilon * epsilon)
        return;

    bool pushedApart = false;

    if (axisMask.x == 1.0f && overlap.x > epsilon)
    {
        if (velocityComponent < 0 && delta.x > 0)
        {
            positionWithOffset.x += overlap.x;
            pushedApart = true;
        }
        else if (velocityComponent > 0 && delta.x < 0)
        {
            positionWithOffset.x -= overlap.x;
            pushedApart = true;
        }
    }
    else if (axisMask.y == 1.0f && overlap.y > epsilon)
    {
        if (velocityComponent < 0 && delta.y > 0)
        {
            positionWithOffset.y += overlap.y;
            pushedApart = true;
        }
        else if (velocityComponent > 0 && delta.y < 0)
        {
            positionWithOffset.y -= overlap.y;
            pushedApart = true;
        }
    }

    // Overlapping while already on the way out is how a body leaves a tile it
    // should never have been inside. Stopping it there strands it for good.
    if (!pushedApart)
        return;

    velocityComponent = 0.0f;
    collisionAABB.expandToInclude(tileAABB);
}

void PhysicsBody::resolveHorizontalCollision(const TileMap &tileMap)
{
    collisionAABBX = AABB();
    glm::vec2 nextPositionWithOffset = nextPosition + getColliderOffset();

    glm::vec2 reducedColliderSize = getColliderSize();
    reducedColliderSize.y *= 0.5f;

    glm::vec2 positionOffset(0.0f);
    positionOffset.y = (getColliderSize().y - reducedColliderSize.y) * 0.5f;

    glm::vec2 probePosition = nextPositionWithOffset + positionOffset;
    AABB proposedAABB(probePosition, reducedColliderSize);

    tileMap.probeSolidTiles(proposedAABB, [&](const AABB &tileAABB)
                            {
        resolveCollisionAgainstTile(
            proposedAABB, 
            tileAABB, 
            {1.0f, 0.0f}, 
            nextVelocity.x, 
            nextPositionWithOffset, 
            collisionAABBX);
        return false; });

    nextPosition = nextPositionWithOffset - getColliderOffset();
}

void PhysicsBody::resolveVerticalCollision(const TileMap &tileMap)
{
    collisionAABBY = AABB();
    glm::vec2 nextPositionWithOffset = nextPosition + getColliderOffset();

    glm::vec2 reducedColliderSize = getColliderSize();
    reducedColliderSize.x *= 0.5f;

    glm::vec2 positionOffset(0.0f);
    positionOffset.x = (getColliderSize().x - reducedColliderSize.x) * 0.5f;

    glm::vec2 probePosition = nextPositionWithOffset + positionOffset;
    AABB proposedAABB(probePosition, reducedColliderSize);

    tileMap.probeSolidTiles(proposedAABB, [&](const AABB &tileAABB)
                            {
        resolveCollisionAgainstTile(
            proposedAABB, 
            tileAABB, 
            {0.0f, 1.0f}, 
            nextVelocity.y, 
            nextPositionWithOffset, 
            collisionAABBY);
        return false; });

    nextPosition = nextPositionWithOffset - getColliderOffset();
}

void PhysicsBody::clampToTileMapBounds(const TileMap &tileMap)
{
    const int mapWidth = tileMap.getWorldWidth();
    const int mapHeight = tileMap.getWorldHeight();

    glm::vec2 clampedPosition = nextPosition + getColliderOffset();
    glm::vec2 clampedVelocity = nextVelocity;
    bool clamped = false;

    AABB playerAABB(clampedPosition, getColliderSize());

    if (playerAABB.left() < 0.0f)
    {
        clampedPosition.x = 0.0f;
        clampedVelocity.x = 0.0f;
        clamped = true;
    }
    else if (playerAABB.right() > mapWidth)
    {
        clampedPosition.x = mapWidth - getColliderSize().x;
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
        clampedPosition.y = mapHeight - getColliderSize().y;
        clampedVelocity.y = 0.0f;
        clamped = true;
    }

    if (clamped)
    {
        nextPosition = clampedPosition - getColliderOffset();
        nextVelocity = clampedVelocity;
    }
}

bool PhysicsBody::contactWithLeftWall(const TileMap &tileMap) const
{
    glm::vec2 probeSize = getColliderSize();
    probeSize.y *= 0.5f;
    glm::vec2 probePosition = position + getColliderOffset() + glm::vec2(-0.1f, 0.0f);
    probePosition.y += (getColliderSize().y - probeSize.y) * 0.5f;
    AABB probeAABB(probePosition, probeSize);
    return tileMap.probeSolidTiles(
        probeAABB,
        [](const AABB &)
        { return true; });
}

bool PhysicsBody::contactWithRightWall(const TileMap &tileMap) const
{
    glm::vec2 probeSize = getColliderSize();
    probeSize.y *= 0.5f;
    glm::vec2 probePosition = position + getColliderOffset() + glm::vec2(0.1f, 0.0f);
    probePosition.y += (getColliderSize().y - probeSize.y) * 0.5f;
    AABB probeAABB(probePosition, probeSize);
    return tileMap.probeSolidTiles(
        probeAABB,
        [](const AABB &)
        { return true; });
}

AABB PhysicsBody::underfootProbe() const
{
    // Just below the feet, not the whole body shifted down. Given the whole
    // body, anything overlapping any part of it reads as ground, so a head
    // stuck in a tile reports standing on it.
    glm::vec2 probeSize(getColliderSize().x * 0.5f, ContactProbeDepth);
    glm::vec2 probePosition = position + getColliderOffset();
    probePosition.x += getColliderSize().x * 0.25f;
    probePosition.y += getColliderSize().y;
    return AABB(probePosition, probeSize);
}

AABB PhysicsBody::overheadProbe() const
{
    glm::vec2 probeSize(getColliderSize().x * 0.5f, ContactProbeDepth);
    glm::vec2 probePosition = position + getColliderOffset();
    probePosition.x += getColliderSize().x * 0.25f;
    probePosition.y -= ContactProbeDepth;
    return AABB(probePosition, probeSize);
}

bool PhysicsBody::contactWithGround(const TileMap &tileMap) const
{
    return tileMap.probeSolidTiles(
               underfootProbe(),
               [](const AABB &)
               { return true; }) ||
           AABB(position + getColliderOffset(), getColliderSize()).bottom() >= tileMap.getWorldHeight();
}

bool PhysicsBody::contactWithCeiling(const TileMap &tileMap) const
{
    return tileMap.probeSolidTiles(
        overheadProbe(),
        [](const AABB &)
        { return true; });
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

const glm::vec2 &PhysicsBody::getColliderOffset() const
{
    return data.colliderOffset;
}

const AABB &PhysicsBody::getCollisionAABBX() const
{
    return collisionAABBX;
}

const AABB &PhysicsBody::getCollisionAABBY() const
{
    return collisionAABBY;
}