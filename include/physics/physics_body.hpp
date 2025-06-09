#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "physics/aabb.hpp"
class TileMap;

class PhysicsBody
{
public:
    void setPosition(glm::vec2 position);
    glm::vec2 getPosition() const;
    void setVelocity(glm::vec2 velocity);
    glm::vec2 getVelocity() const;
    void setColliderSize(glm::vec2 size);
    glm::vec2 getColliderSize() const;
    void setColliderOffset(glm::vec2 offset);
    glm::vec2 getColliderOffset() const;
    void setGravity(float gravity);
    float getGravity() const;
    AABB getAABB() const;
    bool contactInDirection(const TileMap &tileMap, glm::vec2 offset) const;
    bool contactWithLeftWall(const TileMap &tileMap);
    bool contactWithRightWall(const TileMap &tileMap);
    bool contactWithGround(const TileMap &tileMap);
    bool contactWithCeiling(const TileMap &tileMap) const;
    void applyGravity(float deltaTime);
    void stepPhysics(float deltaTime, const TileMap &tileMap);
    AABB getCollisionAABBX() const;
    AABB getCollisionAABBY() const;
    void reset();

private:
    glm::vec2 position = glm::vec2(0, 0);
    glm::vec2 nextPosition = glm::vec2(0, 0);
    glm::vec2 velocity = glm::vec2(0, 0);
    glm::vec2 colliderSize = glm::vec2(16, 16);
    glm::vec2 colliderOffset = glm::vec2(0, 0);
    float gravity = 980;
    AABB collisionAABBX;
    AABB collisionAABBY;

    void resolveCollision(const TileMap &tileMap);
    void clampToTileMapBounds(const TileMap &tileMap);
};