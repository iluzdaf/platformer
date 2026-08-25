#pragma once

#include <signals.hpp>
#include "game/player/player_data.hpp"
#include "game/actor/actor.hpp"
#include "game/actor/behaviors/input_behavior.hpp"
#include "input/input_intentions.hpp"

class Player : public Actor
{
public:
    explicit Player(const PlayerData &data);
    void setInputIntentions(const InputIntentions &inputIntentions);
    void postFixedUpdate() override;

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
    InputBehavior *inputBehavior = nullptr;
};
