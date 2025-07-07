#include <stdexcept>
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

DashAbility::DashAbility(DashAbilityData dashAbilityData)
    : dashAbilityData(dashAbilityData)
{
    if (dashAbilityData.dashSpeed <= 0)
        throw std::invalid_argument("dashSpeed must be > 0");
    if (dashAbilityData.dashDuration <= 0)
        throw std::invalid_argument("dashDuration must be > 0");
}

void DashAbility::fixedUpdate(
    MovementContext &movementContext,
    PlayerState &playerState,
    float deltaTime)
{
    if (playerState.onGround && playerState.dashTimeLeft <= 0.0f)
        playerState.canDash = true;

    if (movementContext.inputIntentions.dashRequested &&
        playerState.canDash &&
        !playerState.touchingLeftWall &&
        !playerState.touchingRightWall)
    {
        playerState.dashDirection = playerState.facingLeft ? -1.0f : 1.0f;
        playerState.dashTimeLeft = dashAbilityData.dashDuration;
        playerState.canDash = false;
        movementContext.emitDash = true;
        playerState.dashing = true;
    }

    if (playerState.dashTimeLeft > 0.0f && playerState.dashing)
    {
        if (playerState.touchingLeftWall || playerState.touchingRightWall)
        {
            playerState.dashTimeLeft = 0.0f;
            playerState.dashing = false;
        }
        else
        {
            playerState.dashTimeLeft -= deltaTime;

            if (playerState.dashTimeLeft > 0.0f)
            {
                movementContext.dashVelocity.x = dashAbilityData.dashSpeed * playerState.dashDirection;
                movementContext.dashVelocity.y = 0.0f;
            }
            else
            {
                playerState.dashTimeLeft = 0.0f;
                playerState.dashing = false;
            }
        }
    }
    else if (playerState.dashTimeLeft <= 0.0f)
        playerState.dashing = false;
}