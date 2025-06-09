#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/player_state.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"

WallSlideAbility::WallSlideAbility(const WallSlideAbilityData &wallSlideAbilityData)
    : slideSpeed(wallSlideAbilityData.slideSpeed),
      hangDuration(wallSlideAbilityData.hangDuration)
{
    assert(slideSpeed > 0);
    assert(hangDuration > 0);
}

void WallSlideAbility::fixedUpdate(
    MovementContext &movementContext,
    const PlayerState &playerState,
    float deltaTime)
{
    wallSliding = false;

    if (playerState.onGround)
    {
        return;
    }

    if (!((moveLeftRequested && playerState.touchingLeftWall) ||
          (moveRightRequested && playerState.touchingRightWall)))
    {
        return;
    }

    hangTime += deltaTime;
    if (hangTime > hangDuration)
    {
        return;
    }

    wallSliding = true;

    const glm::vec2 velocity = movementContext.getVelocity();
    glm::vec2 clampedVelocity = velocity;
    clampedVelocity.y = glm::min(velocity.y, slideSpeed);
    movementContext.setVelocity(clampedVelocity);
    movementContext.emitWallSliding();
}

void WallSlideAbility::update(
    MovementContext & /*movementContext*/,
    const PlayerState &playerState,
    float /*deltaTime*/)
{
    moveLeftRequested = false;
    moveRightRequested = false;

    if (playerState.onGround)
    {
        resetHangTime();
    }
}

void WallSlideAbility::syncState(PlayerState &playerState) const
{
    playerState.wallSliding = wallSliding;
}

float WallSlideAbility::getHangDuration() const
{
    return hangDuration;
}

void WallSlideAbility::resetHangTime()
{
    hangTime = 0;
}

void WallSlideAbility::reset()
{
    wallSliding = false;
    resetHangTime();
}

void WallSlideAbility::tryMoveLeft(
    MovementContext & /*movementContext*/,
    const PlayerState & /*playerState*/)
{
    moveLeftRequested = true;
}

void WallSlideAbility::tryMoveRight(
    MovementContext & /*movementContext*/,
    const PlayerState & /*playerState*/)
{
    moveRightRequested = true;
}