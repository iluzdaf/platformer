#include "game/player/movement_abilities/climb_move_ability.hpp"
#include "game/player/movement_abilities/climb_move_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "game/player/movement_context.hpp"

ClimbMoveAbility::ClimbMoveAbility(const ClimbMoveAbilityData &climbMoveAbilityData)
    : climbSpeed(climbMoveAbilityData.climbSpeed)
{
    if (climbSpeed <= 0)
        throw std::invalid_argument("climbSpeed must be greater than 0");
}

void ClimbMoveAbility::fixedUpdate(
    MovementContext &movementContext,
    const PlayerState & /*playerState*/,
    float /*deltaTime*/)
{
    if (ascendRequested && !descendRequested)
    {
        glm::vec2 velocity = movementContext.getVelocity();
        velocity.y = -climbSpeed;
        movementContext.setVelocity(velocity);
    }
    else if (descendRequested && !ascendRequested)
    {
        glm::vec2 velocity = movementContext.getVelocity();
        velocity.y = climbSpeed;
        movementContext.setVelocity(velocity);
    }
}

void ClimbMoveAbility::update(
    MovementContext & /*movementContext*/,
    const PlayerState & /*playerState*/,
    float /*deltaTime*/)
{
    ascendRequested = false;
    descendRequested = false;
}

void ClimbMoveAbility::tryAscend(
    MovementContext & /*movementContext*/,
    const PlayerState &playerState)
{
    if (playerState.climbing)
        ascendRequested = true;
}

void ClimbMoveAbility::tryDescend(
    MovementContext & /*movementContext*/,
    const PlayerState &playerState)
{
    if (playerState.climbing)
        descendRequested = true;
}

void ClimbMoveAbility::reset()
{
    ascendRequested = false;
    descendRequested = false;
}

float ClimbMoveAbility::getClimbSpeed() const
{
    return climbSpeed;
}