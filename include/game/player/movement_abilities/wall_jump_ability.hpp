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
    void resetJumps();

private:
    int jumpCount = 0;
    float jumpSpeed = -280.0f;
    float horizontalSpeed = 200.0f;
    float wallJumpDuration = 0.25f;
    float wallJumpTimeLeft = 0.0f;
    bool wasTouchingLeftWall = false;
    int wallJumpDirection = 1;
    int maxJumpCount = 2;
};