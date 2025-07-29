#include <stdexcept>
#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

MoveAbility::MoveAbility(MoveAbilityData moveAbilityData)
    : moveAbilityData(moveAbilityData)
{
    if (moveAbilityData.moveSpeed <= 0)
        throw std::runtime_error("moveSpeed must be greater than 0");
}

void MoveAbility::fixedUpdate(
    MovementContext &movementContext,
    PlayerState &,
    float)
{
    if (movementContext.inputIntentions.direction.x > 0)
        movementContext.moveVelocity.x = moveAbilityData.moveSpeed;
    else if (movementContext.inputIntentions.direction.x < 0)
        movementContext.moveVelocity.x = -moveAbilityData.moveSpeed;
}