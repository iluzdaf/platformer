#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class MovementContext;
class DashAbilityData;

class DashAbility : public MovementAbility
{
public:
    explicit DashAbility(const DashAbilityData &dashAbilityData);

    void fixedUpdate(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void update(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void tryDash(MovementContext &movementContext, const PlayerState &playerState) override;
    void syncState(PlayerState &playerState) const override;
    void reset() override;
    
    bool dashing() const;
    float getDashDuration() const;
    float getDashCooldown() const;
    bool canDash() const;

private:
    float dashSpeed = 480;
    float dashDuration = 0.2f;
    float dashCooldown = 0.5f;
    float dashTimeLeft = 0.0f;
    float dashCooldownLeft = 0.0f;
    int dashDirection = 1;
};