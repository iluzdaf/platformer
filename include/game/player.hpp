#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "tile_map.hpp"
#include "animations/sprite_animation.hpp"

enum class PlayerAnimationState
{
    Idle,
    Walk
};

class Player
{
public:
    explicit Player(glm::vec2 startPos);
    void fixedUpdate(float deltaTime, TileMap &tileMap);
    void update(float deltaTime, TileMap &tileMap);
    void jump();
    void moveLeft();
    void moveRight();
    glm::vec2 getPosition() const;
    glm::vec2 getVelocity() const;
    static constexpr float gravity = 980.0f;
    static constexpr float moveSpeed = 160.0f;
    static constexpr float jumpVelocity = -320.0f;
    static constexpr glm::vec2 size = glm::vec2(16, 16);
    const SpriteAnimation &getCurrentAnimation() const;
    PlayerAnimationState getAnimationState() const;
    bool isFacingLeft() const;

private:
    glm::vec2 position;
    glm::vec2 velocity = glm::vec2(0, 0);
    void resolveVerticalCollision(float &nextY, float &velY, const TileMap &tileMap);
    void resolveHorizontalCollision(float &nextX, float &velX, const TileMap &tileMap, float nextY);
    static inline float snapToTileEdge(int tile, int tileSize, bool positive, float entitySize = 0.0f);
    SpriteAnimation idleAnim;
    SpriteAnimation walkAnim;
    SpriteAnimation *currentAnim = nullptr;
    PlayerAnimationState animState = PlayerAnimationState::Idle;
    bool facingLeft = false;
    void updateAnimation(float deltaTime);
    bool isTileSolid(const TileMap &map, int index) const;
    void clampToTileMapBounds(const TileMap &tileMap);
    void handlePickup(TileMap &tilemap);
};