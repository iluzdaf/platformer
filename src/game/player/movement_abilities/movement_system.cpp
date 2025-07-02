#include "game/player/movement_abilities/movement_system.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

void MovementSystem::fixedUpdate(
    PlayerState &playerState,
    InputIntentions inputIntentions,
    float deltaTime)
{
    MovementContext movementContext;
    movementContext.inputIntentions = inputIntentions;
    movementContext.velocity = playerState.targetVelocity;

    for (auto &ability : abilities)
        ability->fixedUpdate(movementContext, playerState, deltaTime);

    playerState.targetVelocity = computeTargetVelocity(movementContext, playerState);

    emitSignals(movementContext);
}

glm::vec2 MovementSystem::computeTargetVelocity(const MovementContext &movementContext, const PlayerState &playerState)
{
    glm::vec2 targetVelocity = movementContext.velocity;

    if (playerState.dashing)
        targetVelocity = movementContext.dashVelocity;
    else
    {
        targetVelocity.x = movementContext.moveVelocity.x;

        if (movementContext.jumpVelocity.y != 0.0f)
            targetVelocity.y = movementContext.jumpVelocity.y;
        else if (playerState.wallJumping)
            targetVelocity = movementContext.wallJumpVelocity;
        else if (playerState.climbing)
            targetVelocity.y = movementContext.climbMoveVelocity.y;
        else if (playerState.wallSliding)
            targetVelocity.y = movementContext.wallSlideVelocity.y;
    }

    return targetVelocity;
}

void MovementSystem::emitSignals(const MovementContext &movementContext)
{
    if (movementContext.emitDash)
        onDash();
    if (movementContext.emitWallJump)
        onWallJump();
    if (movementContext.emitWallSliding)
        onWallSliding();
}

void MovementSystem::addAbility(std::unique_ptr<MovementAbility> ability)
{
    abilities.push_back(std::move(ability));
}

void MovementSystem::clear()
{
    abilities.clear();
}