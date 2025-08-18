#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "physics/aabb.hpp"
#include "physics/physics_body_data.hpp"

class TileMap;

class PhysicsBody
{
public:
    explicit PhysicsBody(const PhysicsBodyData &data);
    void setPosition(const glm::vec2 &position);
    void setVelocity(const glm::vec2 &velocity);
    const glm::vec2 &getPosition() const;
    const glm::vec2 &getVelocity() const;
    const glm::vec2 &getColliderSize() const;
    const glm::vec2 &getColliderOffset() const;
    float getGravity() const;
    AABB getAABB() const;
    bool contactWithLeftWall(const TileMap &tileMap);
    bool contactWithRightWall(const TileMap &tileMap);
    bool contactWithGround(const TileMap &tileMap);
    bool contactWithCeiling(const TileMap &tileMap) const;
    void stepPhysics(float deltaTime, const TileMap &tileMap);
    const AABB& getCollisionAABBX() const;
    const AABB& getCollisionAABBY() const;

private:
    PhysicsBodyData data;
    glm::vec2 position = glm::vec2(0, 0),
              nextPosition = glm::vec2(0, 0),
              velocity = glm::vec2(0, 0),
              nextVelocity = glm::vec2(0, 0);
    float gravity = 980;
    AABB collisionAABBX, collisionAABBY;

    void resolveHorizontalCollision(const TileMap &tileMap);
    void resolveVerticalCollision(const TileMap &tileMap);
    void resolveCollisionAgainstTile(
        const AABB &proposedAABB,
        const AABB &tileAABB,
        const glm::vec2& axisMask,
        float &velocityComponent,
        glm::vec2 &positionWithOffset,
        AABB &collisionAABB);
    void clampToTileMapBounds(const TileMap &tileMap);
};