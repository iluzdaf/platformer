#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/player_state.hpp"
#include "game/player/movement_abilities/movement_context.hpp"

JumpAbility::JumpAbility(JumpAbilityData jumpAbilityData)
    : jumpAbilityData(jumpAbilityData),
      jumpBuffer(jumpAbilityData.jumpBufferDuration),
      coyoteTime(jumpAbilityData.jumpCoyoteDuration)
{
    if (jumpAbilityData.jumpSpeed >= 0)
        throw std::runtime_error("jumpSpeed must be negative");
}

void JumpAbility::fixedUpdate(
    MovementContext &movementContext,
    PlayerState &playerState,
    float deltaTime)
{
    jumpBuffer.update(deltaTime);
    coyoteTime.update(playerState.onGround, deltaTime);

    if (!playerState.jumping)
    {
        if (movementContext.inputIntentions.jumpRequested)
            jumpBuffer.press();

        if (jumpBuffer.isBuffered() &&
            (playerState.onGround || coyoteTime.isCoyoteAvailable()))
        {
            playerState.jumping = true;
            playerState.jumpHoldTime = 0.0f;
            jumpBuffer.consume();
            coyoteTime.consume();
        }
    }

    if (playerState.jumping)
    {
        playerState.jumpHoldTime += deltaTime;
        if (playerState.jumpHoldTime <= jumpAbilityData.jumpDuration &&
            (movementContext.inputIntentions.jumpHeld || movementContext.inputIntentions.jumpRequested))
            movementContext.jumpVelocity.y = jumpAbilityData.jumpSpeed;
        else
            playerState.jumping = false;
    }
}