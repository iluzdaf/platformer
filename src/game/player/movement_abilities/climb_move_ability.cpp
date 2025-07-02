#include "game/player/movement_abilities/climb_move_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

ClimbMoveAbility::ClimbMoveAbility(ClimbMoveAbilityData climbMoveAbilityData)
    : climbMoveAbilityData(climbMoveAbilityData)
{
    if (climbMoveAbilityData.climbSpeed <= 0)
        throw std::invalid_argument("climbSpeed must be greater than 0");
}

void ClimbMoveAbility::fixedUpdate(
    MovementContext &movementContext,
    PlayerState &playerState,
    float /*deltaTime*/)
{
    if (!playerState.climbing)
        return;

    if (movementContext.inputIntentions.direction.y < 0)
        movementContext.climbMoveVelocity.y = -climbMoveAbilityData.climbSpeed;
    else if (movementContext.inputIntentions.direction.y > 0)
        movementContext.climbMoveVelocity.y = climbMoveAbilityData.climbSpeed;
}