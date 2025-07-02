#include "game/player/movement_abilities/climb_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

ClimbAbility::ClimbAbility(ClimbAbilityData climbAbilityData)
    : climbAbilityData(climbAbilityData)
{
}

void ClimbAbility::fixedUpdate(
    MovementContext &movementContext,
    PlayerState &playerState,
    float)
{
    playerState.climbing = false;

    if (!movementContext.inputIntentions.climbRequested)
        return;

    if (!(playerState.touchingLeftWall || playerState.touchingRightWall))
        return;

    playerState.climbing = true;
}