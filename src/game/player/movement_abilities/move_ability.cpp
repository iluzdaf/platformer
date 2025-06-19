#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/movement_abilities/move_ability_data.hpp"
#include "game/player/player_state.hpp"

MoveAbility::MoveAbility(const MoveAbilityData &moveAbilityData)
    : moveSpeed(moveAbilityData.moveSpeed)
{
    if (moveSpeed <= 0)
        throw std::invalid_argument("moveSpeed must be greater than 0");
}

void MoveAbility::fixedUpdate(
    MovementContext &movementContext,
    const PlayerState & /*playerState*/,
    float /*deltaTime*/)
{
    if (moveLeftRequested && !moveRightRequested)
    {
        glm::vec2 velocity = movementContext.getVelocity();
        velocity.x = -moveSpeed;
        movementContext.setVelocity(velocity);
        movementContext.setFacingLeft(true);
    }
    else if (moveRightRequested && !moveLeftRequested)
    {
        glm::vec2 velocity = movementContext.getVelocity();
        velocity.x = moveSpeed;
        movementContext.setVelocity(velocity);
        movementContext.setFacingLeft(false);
    }
}

void MoveAbility::update(
    MovementContext &movementContext,
    const PlayerState & /*playerState*/,
    float /*deltaTime*/)
{
    moveLeftRequested = false;
    moveRightRequested = false;
}

void MoveAbility::tryMoveLeft(
    MovementContext & /*movementContext*/,
    const PlayerState &playerState)
{
    if (playerState.dashing || playerState.wallJumping)
        return;

    moveLeftRequested = true;
}

void MoveAbility::tryMoveRight(
    MovementContext & /*movementContext*/,
    const PlayerState &playerState)
{
    if (playerState.dashing || playerState.wallJumping)
        return;

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

void MoveAbility::reset()
{
    moveLeftRequested = false;
    moveRightRequested = false;
}