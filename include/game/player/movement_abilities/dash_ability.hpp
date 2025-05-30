#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class MovementContext;

class DashAbility : public MovementAbility
{
public:
    DashAbility(float dashSpeed, float dashDuration, float dashCooldown);
    void update(MovementContext &context, float deltaTime) override;
    void tryDash(MovementContext &context) override;
    bool dashing() const;
    float getDashDuration() const;

private:
    float dashSpeed = 480;
    float dashDuration = 0.2f;
    float dashCooldown = 0.5f;
    float dashTimeLeft = 0.0f;
    float dashCooldownLeft = 0.0f;
    int dashDirection = 1;
    bool canDash() const;
};