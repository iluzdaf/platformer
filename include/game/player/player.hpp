#pragma once

#include <signals.hpp>
#include "game/player/player_data.hpp"
#include "game/player/player_state.hpp"
#include "agent/agent.hpp"
#include "animations/animation_manager.hpp"

class TileMap;
struct InputIntentions;

class Player
{
public:
    explicit Player(const PlayerData &data);
    void preFixedUpdate();
    void fixedUpdate(
        float deltaTime,
        const TileMap &tileMap,
        const InputIntentions &inputIntentions);
    const PlayerState &getState() const;
    const Agent &getAgent() const;
    void setPosition(const glm::vec2 &position);

    fteng::signal<void()>
        onLevelComplete,
        onDeath,
        onFallFromHeight,
        onHitCeiling,
        onWallJump,
        onDash,
        onWallSliding;
    fteng::signal<void(int)>
        onPickup;

private:
    PlayerData data;
    Agent agent;
    PlayerState playerState;
    AnimationManager animationManager;

    void updateState();
    void emitSignals();
};