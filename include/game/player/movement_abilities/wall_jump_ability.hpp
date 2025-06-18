#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class WallJumpAbilityData;

class WallJumpAbility : public MovementAbility
{
public:
    explicit WallJumpAbility(const WallJumpAbilityData &wallJumpAbilityData);

    void fixedUpdate(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void update(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void tryJump(MovementContext &MovementContext, const PlayerState &playerState) override;
    void syncState(PlayerState &playerState) const override;
    void reset() override;

    bool wallJumping() const;

private:
    int jumpCount = 0,
        wallJumpDirection = 1,
        maxJumpCount = 2;
    float jumpSpeed = -280.0f,
          horizontalSpeed = 200.0f,
          wallJumpDuration = 0.25f,
          wallJumpTimeLeft = 0.0f,
          wallJumpBufferTime = 0,
          wallJumpBufferDuration = 0.1f;
    bool wasTouchingLeftWall = false;

    void resetJumps();
    void performWallJump(
        MovementContext &movementContext,
        const PlayerState &playerState);
};