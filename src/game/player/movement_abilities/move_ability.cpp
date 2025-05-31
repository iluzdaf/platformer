#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/movement_abilities/move_ability_data.hpp"
#include "game/player/player_state.hpp"

MoveAbility::MoveAbility(const MoveAbilityData &moveAbilityData)
    : moveSpeed(moveAbilityData.moveSpeed)
{
}

void MoveAbility::update(MovementContext &movementContext, float /*deltaTime*/)
{
    if (moveLeftRequested && !moveRightRequested)
    {
        glm::vec2 vel = movementContext.getVelocity();
        vel.x = -moveSpeed;
        movementContext.setVelocity(vel);
        movementContext.setFacingLeft(true);
    }
    else if (moveRightRequested && !moveLeftRequested)
    {
        glm::vec2 vel = movementContext.getVelocity();
        vel.x = moveSpeed;
        movementContext.setVelocity(vel);
        movementContext.setFacingLeft(false);
    }

    moveLeftRequested = false;
    moveRightRequested = false;
}

void MoveAbility::tryMoveLeft(MovementContext &context)
{
    moveLeftRequested = true;
}

void MoveAbility::tryMoveRight(MovementContext &context)
{
    moveRightRequested = true;
}

float MoveAbility::getMoveSpeed() const
{
    return moveSpeed;
}

void MoveAbility::syncState(PlayerState &playerState) const
{
    playerState.moveSpeed = moveSpeed;
}