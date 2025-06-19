#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class MovementContext;
class JumpAbilityData;

class JumpAbility : public MovementAbility
{
public:
    explicit JumpAbility(const JumpAbilityData &jumpAbilityData);

    void fixedUpdate(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void update(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void tryJump(MovementContext &movementContext, const PlayerState &playerState) override;
    void syncState(PlayerState &playerState) const override;
    void reset() override;

    int getMaxJumpCount() const;
    float getJumpSpeed() const;
    int getJumpCount() const;

private:
    int maxJumpCount = 2,
        jumpCount = 0;
    float jumpSpeed = -280,
          jumpBufferTime = 0,
          jumpBufferDuration = 0.1f,
          jumpCoyoteDuration = 0.2f,
          jumpCoyoteTime = 0;

    void resetJumps();
    void performJump(MovementContext &movementContext);
};