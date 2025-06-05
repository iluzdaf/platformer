#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "physics/aabb.hpp"
class TileMap;

class PhysicsBody
{
public:
    PhysicsBody() = default;
    void setPosition(glm::vec2 position);
    glm::vec2 getPosition() const;
    void setVelocity(glm::vec2 velocity);
    glm::vec2 getVelocity() const;
    void setSize(glm::vec2 size);
    glm::vec2 getSize() const;
    void setOffset(glm::vec2 offset);
    glm::vec2 getOffset() const;
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

private:
    glm::vec2 position = glm::vec2(0, 0);
    glm::vec2 nextPosition = glm::vec2(0, 0);
    glm::vec2 velocity = glm::vec2(0, 0);
    glm::vec2 size = glm::vec2(16, 16);
    float gravity = 980;
    glm::vec2 offset = glm::vec2(0, 0);

    void resolveCollision(const TileMap &tileMap);
    void clampToTileMapBounds(const TileMap &tileMap);
};