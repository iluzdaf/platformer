#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "physics/aabb.hpp"
#include "physics/physics_body_data.hpp"

class TileMap;

class PhysicsBody
{
public:
    explicit PhysicsBody(const PhysicsBodyData &data);
    void setPosition(const glm::vec2 &newPosition);
    void setVelocity(const glm::vec2 &newVelocity);
    const glm::vec2 &position() const;
    const glm::vec2 &velocity() const;
    const glm::vec2 &colliderSize() const;
    const glm::vec2 &colliderOffset() const;
    AABB aabb() const;
    AABB touchBox() const;
    glm::vec2 bottomCenterOffset() const;
    bool contactWithLeftWall(const TileMap &tileMap) const;
    bool contactWithRightWall(const TileMap &tileMap) const;
    bool gripOnLeftWall(const TileMap &tileMap) const;
    bool gripOnRightWall(const TileMap &tileMap) const;
    bool contactWithLeftWallAtHead(const TileMap &tileMap) const;
    bool contactWithRightWallAtHead(const TileMap &tileMap) const;
    AABB wallProbe(float side) const;
    AABB wallProbeAtHead(float side) const;
    AABB underfootProbe() const;
    AABB overheadProbe() const;
    bool contactWithGround(const TileMap &tileMap) const;
    bool contactWithCeiling(const TileMap &tileMap) const;
    void stepPhysics(float deltaTime, const TileMap &tileMap);
    const AABB &collisionAABBX() const;
    const AABB &collisionAABBY() const;

private:
    PhysicsBodyData data;
    struct Motion
    {
        glm::vec2 position = glm::vec2(0, 0), velocity = glm::vec2(0, 0);
    };
    Motion now, next;
    AABB collisionX, collisionY;

    void resolveHorizontalCollision(const TileMap &tileMap);
    void resolveVerticalCollision(const TileMap &tileMap);
    void resolveCollisionAgainstTile(
        const AABB &proposedAABB,
        const AABB &tileAABB,
        const glm::vec2 &axisMask,
        float &velocityComponent,
        glm::vec2 &positionWithOffset,
        AABB &collisionAABB);
    AABB horizontalProbeAt(glm::vec2 positionWithOffset) const;
    AABB verticalProbeAt(glm::vec2 positionWithOffset) const;
    void pushOutOfSolids(const TileMap &tileMap);
    void clampToTileMapBounds(const TileMap &tileMap);
};