#include <algorithm>
#include <functional>
#include <vector>
#include <stdexcept>
#include <string>
#include "physics/physics_body.hpp"
#include "physics/physics_body_data.hpp"
#include "physics/aabb.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile.hpp"

namespace
{
    constexpr float ContactProbeDepth = 0.1f;
    constexpr float HeadroomFraction = 0.25f;
}

PhysicsBody::PhysicsBody(const PhysicsBodyData &data) : data(data)
{
    if (data.colliderSize.x <= 0.0f || data.colliderSize.y <= 0.0f)
        throw std::runtime_error("A collider of no size is not one anything can touch");

    if (data.stepHeight < 0.0f)
        throw std::runtime_error("A step height below 0 is not a height");

    if (data.stepHeight >= data.colliderSize.y * (1.0f - HeadroomFraction))
        throw std::runtime_error(
            "A step height of " + std::to_string(data.stepHeight) +
            " leaves nothing of the body to walk into a wall with");
}

void PhysicsBody::setPosition(const glm::vec2 &newPosition)
{
    now.position = newPosition;
}

glm::vec2 PhysicsBody::position() const
{
    return now.position;
}

void PhysicsBody::setVelocity(const glm::vec2 &newVelocity)
{
    now.velocity = newVelocity;
}

glm::vec2 PhysicsBody::velocity() const
{
    return now.velocity;
}

glm::vec2 PhysicsBody::colliderSize() const
{
    return data.colliderSize;
}

AABB PhysicsBody::aabb() const
{
    return AABB(now.position + colliderOffset(), colliderSize());
}

AABB PhysicsBody::touchBox() const
{
    glm::vec2 skin(ContactProbeDepth);
    return AABB(now.position + colliderOffset() - skin, colliderSize() + skin * 2.0f);
}

glm::vec2 PhysicsBody::bottomCenterOffset() const
{
    return colliderOffset() + glm::vec2(colliderSize().x * 0.5f, colliderSize().y);
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

    if (!pushedApart)
        return;

    velocityComponent = 0.0f;
    collisionAABB.expandToInclude(tileAABB);
}

AABB PhysicsBody::horizontalProbeAt(glm::vec2 positionWithOffset) const
{
    float headroom = colliderSize().y * HeadroomFraction;
    glm::vec2 size = colliderSize();
    size.y -= headroom + data.stepHeight;

    return AABB(positionWithOffset + glm::vec2(0.0f, headroom), size);
}

AABB PhysicsBody::verticalProbeAt(glm::vec2 positionWithOffset) const
{
    glm::vec2 size = colliderSize();
    size.x *= 0.5f;

    return AABB(positionWithOffset + glm::vec2((colliderSize().x - size.x) * 0.5f, 0.0f), size);
}

void PhysicsBody::resolveHorizontalCollision(const TileMap &tileMap)
{
    collisionX = AABB();
    glm::vec2 nextPositionWithOffset = next.position + colliderOffset();

    tileMap.probeSolidTiles(
        horizontalProbeAt(nextPositionWithOffset),
        [&](const Tile &, const AABB &tileAABB)
        {
            resolveCollisionAgainstTile(
                horizontalProbeAt(nextPositionWithOffset),
                tileAABB,
                {1.0f, 0.0f},
                next.velocity.x,
                nextPositionWithOffset,
                collisionX);
            return false;
        });

    next.position = nextPositionWithOffset - colliderOffset();
}

void PhysicsBody::resolveVerticalCollision(const TileMap &tileMap)
{
    collisionY = AABB();
    glm::vec2 nextPositionWithOffset = next.position + colliderOffset();

    tileMap.probeSolidTiles(
        verticalProbeAt(nextPositionWithOffset),
        [&](const Tile &, const AABB &tileAABB)
        {
            resolveCollisionAgainstTile(
                verticalProbeAt(nextPositionWithOffset),
                tileAABB,
                {0.0f, 1.0f},
                next.velocity.y,
                nextPositionWithOffset,
                collisionY);
            return false;
        });

    next.position = nextPositionWithOffset - colliderOffset();
}

void PhysicsBody::pushOutOfSolids(const TileMap &tileMap)
{
    glm::vec2 nextPositionWithOffset = next.position + colliderOffset();
    const float epsilon = 0.001f;

    auto insideTheMap = [&](glm::vec2 moved)
    {
        AABB body(nextPositionWithOffset + moved, colliderSize());
        return body.left() >= 0.0f && body.top() >= 0.0f &&
               body.right() <= static_cast<float>(tileMap.getWorldWidth()) &&
               body.bottom() <= static_cast<float>(tileMap.getWorldHeight());
    };

    struct WayOut
    {
        glm::vec2 move;
        float *velocityComponent;
        AABB *collisionAABB;
    };

    auto waysOutOf = [&](const AABB &tileAABB, std::vector<WayOut> &ways)
    {
        auto along = [&](const AABB &probe, glm::vec2 axis, float &speed, AABB &box)
        {
            glm::vec2 delta = probe.center() - tileAABB.center();
            glm::vec2 overlap = (tileAABB.size + probe.size) * 0.5f - glm::abs(delta);
            if (overlap.x < epsilon || overlap.y < epsilon)
                return;

            ways.push_back(
                {-axis * (probe.position + probe.size - tileAABB.position), &speed, &box});
            ways.push_back(
                {axis * (tileAABB.position + tileAABB.size - probe.position), &speed, &box});
        };

        along(verticalProbeAt(nextPositionWithOffset), {0.0f, 1.0f}, next.velocity.y, collisionY);
        along(horizontalProbeAt(nextPositionWithOffset), {1.0f, 0.0f}, next.velocity.x, collisionX);
    };

    tileMap.probeSolidTiles(
        AABB(nextPositionWithOffset, colliderSize()),
        [&](const Tile &, const AABB &tileAABB)
        {
            std::vector<WayOut> ways;
            waysOutOf(tileAABB, ways);
            if (ways.empty())
                return false;

            auto shorter = [](const WayOut &a, const WayOut &b)
            { return glm::length(a.move) < glm::length(b.move); };
            std::sort(ways.begin(), ways.end(), shorter);
            auto allowed = std::find_if(
                ways.begin(),
                ways.end(),
                [&](const WayOut &way) { return insideTheMap(way.move); });
            const WayOut &taken = allowed == ways.end() ? ways.front() : *allowed;

            nextPositionWithOffset += taken.move;
            *taken.velocityComponent = 0.0f;
            taken.collisionAABB->expandToInclude(tileAABB);
            return false;
        });

    next.position = nextPositionWithOffset - colliderOffset();
}

void PhysicsBody::clampToTileMapBounds(const TileMap &tileMap)
{
    const int mapWidth = tileMap.getWorldWidth();
    const int mapHeight = tileMap.getWorldHeight();

    glm::vec2 clampedPosition = next.position + colliderOffset();
    glm::vec2 clampedVelocity = next.velocity;
    bool clamped = false;

    AABB playerAABB(clampedPosition, colliderSize());

    if (playerAABB.left() < 0.0f)
    {
        clampedPosition.x = 0.0f;
        clampedVelocity.x = 0.0f;
        clamped = true;
    }
    else if (playerAABB.right() > mapWidth)
    {
        clampedPosition.x = mapWidth - colliderSize().x;
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
        clampedPosition.y = mapHeight - colliderSize().y;
        clampedVelocity.y = 0.0f;
        clamped = true;
    }

    if (clamped)
    {
        next.position = clampedPosition - colliderOffset();
        next.velocity = clampedVelocity;
    }
}

AABB PhysicsBody::wallProbe(float side) const
{
    glm::vec2 probeSize = colliderSize();
    probeSize.y *= 0.5f;
    glm::vec2 probePosition = now.position + colliderOffset() + glm::vec2(side * 0.1f, 0.0f);
    probePosition.y += (colliderSize().y - probeSize.y) * 0.5f;
    return AABB(probePosition, probeSize);
}

bool PhysicsBody::contactWithLeftWall(const TileMap &tileMap) const
{
    return tileMap.probeSolidTiles(
        wallProbe(-1.0f), [](const Tile &, const AABB &) { return true; });
}

bool PhysicsBody::contactWithRightWall(const TileMap &tileMap) const
{
    return tileMap.probeSolidTiles(
        wallProbe(1.0f), [](const Tile &, const AABB &) { return true; });
}

bool PhysicsBody::gripOnLeftWall(const TileMap &tileMap) const
{
    return tileMap.probeSolidTiles(
        wallProbe(-1.0f), [](const Tile &tile, const AABB &) { return tile.isGrippable(); });
}

bool PhysicsBody::gripOnRightWall(const TileMap &tileMap) const
{
    return tileMap.probeSolidTiles(
        wallProbe(1.0f), [](const Tile &tile, const AABB &) { return tile.isGrippable(); });
}

AABB PhysicsBody::wallProbeAtHead(float side) const
{
    glm::vec2 probeSize = colliderSize();
    probeSize.y *= 0.25f;
    glm::vec2 probePosition = now.position + colliderOffset() + glm::vec2(side * 0.1f, 0.0f);
    return AABB(probePosition, probeSize);
}
bool PhysicsBody::contactWithLeftWallAtHead(const TileMap &tileMap) const
{
    return tileMap.probeSolidTiles(
        wallProbeAtHead(-1.0f), [](const Tile &, const AABB &) { return true; });
}

bool PhysicsBody::contactWithRightWallAtHead(const TileMap &tileMap) const
{
    return tileMap.probeSolidTiles(
        wallProbeAtHead(1.0f), [](const Tile &, const AABB &) { return true; });
}

AABB PhysicsBody::underfootProbe() const
{
    glm::vec2 probeSize(colliderSize().x * 0.5f, ContactProbeDepth);
    glm::vec2 probePosition = now.position + colliderOffset();
    probePosition.x += colliderSize().x * 0.25f;
    probePosition.y += colliderSize().y;
    return AABB(probePosition, probeSize);
}

AABB PhysicsBody::overheadProbe() const
{
    glm::vec2 probeSize(colliderSize().x * 0.5f, ContactProbeDepth);
    glm::vec2 probePosition = now.position + colliderOffset();
    probePosition.x += colliderSize().x * 0.25f;
    probePosition.y -= ContactProbeDepth;
    return AABB(probePosition, probeSize);
}

bool PhysicsBody::contactWithGround(const TileMap &tileMap) const
{
    return tileMap.probeSolidTiles(
               underfootProbe(), [](const Tile &, const AABB &) { return true; }) ||
           AABB(now.position + colliderOffset(), colliderSize()).bottom() >=
               tileMap.getWorldHeight();
}

bool PhysicsBody::contactWithCeiling(const TileMap &tileMap) const
{
    return tileMap.probeSolidTiles(
        overheadProbe(), [](const Tile &, const AABB &) { return true; });
}

void PhysicsBody::stepPhysics(float deltaTime, const TileMap &tileMap)
{
    next.position = now.position;
    next.velocity = now.velocity;

    next.position += glm::vec2(next.velocity.x, 0) * deltaTime;
    resolveHorizontalCollision(tileMap);

    next.position += glm::vec2(0, next.velocity.y) * deltaTime;
    resolveVerticalCollision(tileMap);

    pushOutOfSolids(tileMap);
    clampToTileMapBounds(tileMap);

    now.position = next.position;
    now.velocity = next.velocity;
}

glm::vec2 PhysicsBody::colliderOffset() const
{
    return data.colliderOffset;
}

AABB PhysicsBody::collisionAABBX() const
{
    return collisionX;
}

AABB PhysicsBody::collisionAABBY() const
{
    return collisionY;
}