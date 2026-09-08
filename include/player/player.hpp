#pragma once

#include <signals.hpp>
#include "player/player_data.hpp"
#include "actor/actor.hpp"
#include "input/intention_source.hpp"

class Player : public Actor
{
public:
    Player(const PlayerData &data, const IntentionSource &intentionSource);
    void postFixedUpdate() override;
    void completeLevel();
    fteng::signal<void()> onLevelComplete, onFallFromHeight, onHitCeiling, onWallJump, onDash,
        onWallSliding;

private:
    PlayerData data;
    bool levelCompleted = false;
};
