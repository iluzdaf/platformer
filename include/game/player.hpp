#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "tile_map.hpp"

class Player {
public:
    Player(glm::vec2 startPos, glm::vec2 size = glm::vec2(16, 16));
    void update(float deltaTime, const TileMap& tileMap);
    void jump();
    void moveLeft();
    void moveRight();
    glm::vec2 getPosition() const;
    glm::vec2 getSize() const;
    glm::vec2 getVelocity() const;
    static constexpr float gravity = 980.0f;
    static constexpr float moveSpeed = 200.0f;
    static constexpr float jumpVelocity = -400.0f;

private:
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 size;
    void resolveVerticalCollision(float& nextY, float& velY, const TileMap& tileMap, const glm::vec2& size);
    void resolveHorizontalCollision(float& nextX, float& velX, const TileMap& tileMap, const glm::vec2& size, float nextY);
    static inline float snapToTileEdge(int tile, int tileSize, bool positive, float entitySize = 0.0f);
};