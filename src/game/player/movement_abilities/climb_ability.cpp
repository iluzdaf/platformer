#include "game/player/movement_abilities/climb_ability.hpp"
#include "game/player/movement_abilities/climb_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "game/player/movement_context.hpp"

ClimbAbility::ClimbAbility(const ClimbAbilityData &climbAbilityData)
    : climbDuration(climbAbilityData.climbDuration)
{
    if (climbDuration <= 0)
        throw std::invalid_argument("climbDuration must be greater than 0");
}

void ClimbAbility::fixedUpdate(
    MovementContext &movementContext,
    const PlayerState &playerState,
    float deltaTime)
{
    climbing = false;

    if (!climbRequested)
        return;

    if (!(playerState.touchingLeftWall || playerState.touchingRightWall))
        return;

    if (playerState.wallJumping)
        return;

    climbTime += deltaTime;
    if (climbTime > climbDuration)
        return;

    climbing = true;

    glm::vec2 velocity = movementContext.getVelocity();
    velocity.y = 0;
    movementContext.setVelocity(velocity);
}

void ClimbAbility::update(
    MovementContext & /*movementContext*/,
    const PlayerState &playerState,
    float /*deltaTime*/)
{
    climbRequested = false;

    if (playerState.onGround)
        climbTime = 0.0f;
}

void ClimbAbility::tryClimb(
    MovementContext & /*movementContext*/,
    const PlayerState & /*playerState*/)
{
    climbRequested = true;
}

void ClimbAbility::syncState(PlayerState &playerState) const
{
    playerState.climbing = climbing;
}

void ClimbAbility::reset()
{
    climbTime = 0;
    climbRequested = false;
    climbing = false;
}

float ClimbAbility::getClimbDuration() const
{
    return climbDuration;
}