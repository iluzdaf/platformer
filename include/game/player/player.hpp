#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "game/tile_map/tile_map.hpp"
#include "game/player/player_data.hpp"
#include "game/player/player_animation_state.hpp"
#include "game/physics_data.hpp"
#include "animations/sprite_animation.hpp"

class Player
{
public:
    Player(const PlayerData &playerData, const PhysicsData &physicsData);
    void fixedUpdate(float deltaTime, TileMap &tileMap);
    void update(float deltaTime, TileMap &tileMap);
    void jump();
    void moveLeft();
    void moveRight();
    void dash();
    glm::vec2 getPosition() const;
    glm::vec2 getVelocity() const;
    const SpriteAnimation &getCurrentAnimation() const;
    PlayerAnimationState getAnimationState() const;
    glm::vec2 getSize() const;
    float getMoveSpeed() const;
    float getJumpSpeed() const;
    int getMaxJumpCount() const;
    float getDashDuration() const;
    bool facingLeft() const;
    bool dashing() const;
    bool canDash() const;

private:
    void resolveVerticalCollision(float &nextY, float &velY, const TileMap &tileMap);
    void resolveHorizontalCollision(float &nextX, float &velX, const TileMap &tileMap, float nextY);
    static inline float snapToTileEdge(int tile, int tileSize, bool positive, float entitySize = 0.0f);
    void updateAnimation(float deltaTime);
    void clampToTileMapBounds(const TileMap &tileMap);
    void handlePickup(TileMap &tilemap);
    glm::vec2 position = glm::vec2(0, 0);
    glm::vec2 velocity = glm::vec2(0, 0);
    SpriteAnimation idleAnim;
    SpriteAnimation walkAnim;
    SpriteAnimation *currentAnim = nullptr;
    PlayerAnimationState animState = PlayerAnimationState::Idle;
    bool isFacingLeft = false;
    float moveSpeed = 0, jumpSpeed = 0;
    glm::vec2 size = glm::vec2(1, 1);
    float gravity = 980;
    int jumpCount = 0, maxJumpCount = 2;
    bool onGround = false;
    float dashTimeLeft = 0.0f;
    float dashCooldownLeft = 0.0f;
    float dashSpeed = 480.0f;
    float dashDuration = 0.2f;
    float dashCooldown = 1.0f;
    int dashDirection = 1;
};