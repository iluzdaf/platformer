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
    fteng::signal<void()> onLevelComplete, onDeath, onHurt, onFallFromHeight, onHitCeiling,
        onWallJump, onDash, onWallSliding;

private:
    void hurt() override;
    void died() override;
    PlayerData data;
    bool levelCompleted = false;
};
