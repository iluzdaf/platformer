#include "game/player/movement_abilities/wall_jump_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

WallJumpAbility::WallJumpAbility(WallJumpAbilityData wallJumpAbilityData)
    : wallJumpAbilityData(wallJumpAbilityData),
      wallJumpBuffer(wallJumpAbilityData.wallJumpBufferDuration),
      wallJumpCoyote(wallJumpAbilityData.wallJumpCoyoteDuration)
{
    if (wallJumpAbilityData.wallJumpSpeed >= 0)
        throw std::runtime_error("wallJumpSpeed must be negative");
    if (wallJumpAbilityData.wallJumpHorizontalSpeed <= 0)
        throw std::runtime_error("wallJumpHorizontalSpeed must be greater than 0");
}

void WallJumpAbility::fixedUpdate(
    MovementContext &movementContext,
    PlayerState &playerState,
    float deltaTime)
{
    wallJumpBuffer.update(deltaTime);
    wallJumpCoyote.update(playerState.touchingLeftWall || playerState.touchingRightWall, deltaTime);

    if (playerState.onGround)
    {
        wallJumpCoyote.consume();
        return;
    }

    if (!playerState.wallJumping)
    {
        if (movementContext.inputIntentions.jumpHeld)
        {
            wallJumpBuffer.press();
            wallJumpDirectionBuffer.press(movementContext.inputIntentions.direction.x);
        }

        if (wallJumpBuffer.isBuffered())
        {
            int desiredDirection = 0;
            if (playerState.touchingLeftWall)
                desiredDirection = 1;
            else if (playerState.touchingRightWall)
                desiredDirection = -1;
            else
                desiredDirection = playerState.wasLastWallLeft ? 1 : -1;

            float bufferedDirection = wallJumpDirectionBuffer.getBufferedDirectionX();
            bool jumpInputCorrect = desiredDirection * bufferedDirection > 0;
            bool touchingWallNow = playerState.touchingLeftWall || playerState.touchingRightWall;
            if (touchingWallNow && jumpInputCorrect)
                startWallJump(movementContext, playerState, desiredDirection);
            else if (!touchingWallNow && wallJumpCoyote.isCoyoteAvailable() && jumpInputCorrect)
                startWallJump(movementContext, playerState, desiredDirection);
        }
    }

    if (playerState.wallJumping)
    {
        bool switchedSides =
            (playerState.touchingLeftWall && playerState.wallJumpDirection == -1) ||
            (playerState.touchingRightWall && playerState.wallJumpDirection == 1);

        if (switchedSides)
        {
            playerState.wallJumpTimeLeft = 0.0f;
            playerState.wallJumping = false;
            return;
        }

        playerState.wallJumpTimeLeft -= deltaTime;
        if (playerState.wallJumpTimeLeft <= 0.0f)
        {
            playerState.wallJumping = false;
            return;
        }

        movementContext.wallJumpVelocity = {
            wallJumpAbilityData.wallJumpHorizontalSpeed * playerState.wallJumpDirection,
            wallJumpAbilityData.wallJumpSpeed};
    }
}

void WallJumpAbility::startWallJump(
    MovementContext &movementContext,
    PlayerState &playerState,
    int direction)
{
    playerState.wallJumpDirection = static_cast<float>(direction);
    playerState.wallJumpTimeLeft = wallJumpAbilityData.wallJumpDuration;
    playerState.wallJumping = true;
    movementContext.emitWallJump = true;
    wallJumpBuffer.consume();
    wallJumpDirectionBuffer.consume();
    wallJumpCoyote.consume();
}