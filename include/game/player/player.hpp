#pragma once
#include <memory>
#include <vector>
#include <signals.hpp>
#include "game/player/player_animation_state.hpp"
#include "game/player/movement_abilities/movement_ability.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/player_state.hpp"
#include "animations/sprite_animation.hpp"
#include "physics/physics_body.hpp"
class TileMap;
class PlayerData;
class PhysicsData;

class Player : public MovementContext
{
public:
    Player(const PlayerData &playerData, const PhysicsData &physicsData);
    void preFixedUpdate();
    void fixedUpdate(float deltaTime, TileMap &tileMap);
    void update(float deltaTime, TileMap &tileMap);
    void jump();
    void moveLeft();
    void moveRight();
    void dash();
    SpriteAnimation &getCurrentAnimation();
    PlayerAnimationState getAnimationState() const;
    glm::vec2 getSize() const;
    bool facingLeft() const;
    const PlayerState &getPlayerState() const;
    void setPosition(const glm::vec2 &position);
    glm::vec2 getPosition() const;
    AABB getAABB() const;
    void reset();

    glm::vec2 getVelocity() const override;
    void setVelocity(const glm::vec2 &velocity) override;
    void setFacingLeft(bool isFacingLeft) override;

    fteng::signal<void()> onLevelComplete;
    fteng::signal<void()> onDeath;

private:
    void updatePlayerState(const TileMap &tileMap);

    PhysicsBody physicsBody;
    bool isFacingLeft = false;
    glm::vec2 size = glm::vec2(16, 16);
    std::vector<std::unique_ptr<MovementAbility>> movementAbilities;
    PlayerState playerState;
    
    void updateAnimation(float deltaTime);
    SpriteAnimation idleAnim;
    SpriteAnimation walkAnim;
    PlayerAnimationState animState = PlayerAnimationState::Idle;
};