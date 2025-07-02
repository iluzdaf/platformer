#pragma once
#include <memory>
#include <vector>
#include <signals.hpp>
#include "game/player/movement_abilities/movement_system.hpp"
#include "game/player/player_state.hpp"
#include "input/input_intentions.hpp"
#include "physics/physics_body.hpp"
#include "animations/animation_manager.hpp"

class TileMap;
struct PlayerData;
struct PhysicsData;

class Player
{
public:
    Player(PlayerData playerData, PhysicsData physicsData);
    void preFixedUpdate();
    void fixedUpdate(float deltaTime, TileMap &tileMap, InputIntentions inputIntentions);
    const PlayerState &getPlayerState() const;
    void setPosition(const glm::vec2 &position);
    AABB getAABB() const;
    void reset();
    void initFromData(PlayerData playerData, PhysicsData physicsData);
    MovementSystem &getMovementSystem();

    fteng::signal<void()>
        onLevelComplete,
        onDeath,
        onFallFromHeight,
        onHitCeiling;
    fteng::signal<void(int)> onPickup;

private:
    glm::vec2 size = glm::vec2(16, 16);
    float fallFromHeightThreshold = 600;
    PlayerState playerState;
    MovementSystem movementSystem;
    PhysicsBody physicsBody;
    AnimationManager animationManager;

    void updatePlayerState();
    void updatePlayerPhysicsState(const TileMap &tileMap);
};