#pragma once

#include <signals.hpp>
#include "game/player/player_data.hpp"
#include "game/actor/actor.hpp"
#include "game/actor/behaviors/input_behavior.hpp"
#include "input/intention_source.hpp"

class Player : public Actor
{
public:
    Player(const PlayerData &data, const IntentionSource &intentionSource);
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
};
