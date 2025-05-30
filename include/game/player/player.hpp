#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include "game/player/player_animation_state.hpp"
#include "game/player/movement_abilities/movement_ability.hpp"
#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_context.hpp"
#include "animations/sprite_animation.hpp"
class TileMap;
class PlayerData;
class PhysicsData;

class Player : public MovementContext
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
    float getDashDuration() const;
    bool facingLeft() const;
    bool dashing() const;
    bool canDash() const;
    bool onGround() const;
    JumpAbility* getJumpAbility();
    void setOnGround(bool isOnGround);
    void setVelocity(const glm::vec2& velocity);

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
    float moveSpeed = 0;
    glm::vec2 size = glm::vec2(1, 1);
    float gravity = 980;
    bool isOnGround = false;
    float dashTimeLeft = 0.0f;
    float dashCooldownLeft = 0.0f;
    float dashSpeed = 480.0f;
    float dashDuration = 0.2f;
    float dashCooldown = 1.0f;
    int dashDirection = 1;
    std::vector<std::unique_ptr<MovementAbility>> movementAbilities;
};