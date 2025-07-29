#include <stdexcept>
#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

WallSlideAbility::WallSlideAbility(WallSlideAbilityData wallSlideAbilityData)
    : wallSlideAbilityData(wallSlideAbilityData)
{
    if (wallSlideAbilityData.slideSpeed <= 0)
        throw std::runtime_error("slideSpeed must be positive");
}

void WallSlideAbility::fixedUpdate(
    MovementContext &movementContext,
    PlayerState &playerState,
    float)
{
    playerState.wallSliding = false;

    if (playerState.onGround ||
        !(playerState.touchingLeftWall || playerState.touchingRightWall) ||
        playerState.velocity.y <= 0.0f)
        return;

    movementContext.wallSlideVelocity.y = wallSlideAbilityData.slideSpeed;
    playerState.wallSliding = true;
    movementContext.emitWallSliding = true;
}