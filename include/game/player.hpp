#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "tile_map.hpp"
#include "animations/sprite_animation.hpp"
#include "game/player_data.hpp"
#include "game/player_animation_state.hpp"
#include "game/physics_data.hpp"

class Player
{
public:
    Player(const PlayerData &playerData, const PhysicsData &physicsData);
    void fixedUpdate(float deltaTime, TileMap &tileMap);
    void update(float deltaTime, TileMap &tileMap);
    void jump();
    void moveLeft();
    void moveRight();
    glm::vec2 getPosition() const;
    glm::vec2 getVelocity() const;
    const SpriteAnimation &getCurrentAnimation() const;
    PlayerAnimationState getAnimationState() const;
    bool isFacingLeft() const;
    glm::vec2 getSize() const;
    float getMoveSpeed() const;
    float getJumpSpeed() const;

private:
    glm::vec2 position = glm::vec2(0, 0);
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
    float moveSpeed = 0, jumpSpeed = 0;
    glm::vec2 size = glm::vec2(1, 1);
    void initFromData(const PlayerData &playerData);
    float gravity;
};