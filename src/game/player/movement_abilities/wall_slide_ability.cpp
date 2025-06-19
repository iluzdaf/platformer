#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/player_state.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"

WallSlideAbility::WallSlideAbility(const WallSlideAbilityData &wallSlideAbilityData)
    : slideSpeed(wallSlideAbilityData.slideSpeed)
{
    if (slideSpeed <= 0)
        throw std::invalid_argument("slideSpeed must be greater than 0");
}

void WallSlideAbility::fixedUpdate(
    MovementContext &movementContext,
    const PlayerState &playerState,
    float /*deltaTime*/)
{
    wallSliding = false;

    if (playerState.onGround)
        return;

    if (playerState.climbing)
        return;

    if (!(playerState.touchingLeftWall || playerState.touchingRightWall))
        return;

    glm::vec2 velocity = movementContext.getVelocity();

    if (velocity.y <= 0)
        return;

    wallSliding = true;
    velocity.y = slideSpeed;
    movementContext.setVelocity(velocity);
    movementContext.emitWallSliding();
}

void WallSlideAbility::update(
    MovementContext & /*movementContext*/,
    const PlayerState & /*playerState*/,
    float /*deltaTime*/)
{
}

void WallSlideAbility::syncState(PlayerState &playerState) const
{
    playerState.wallSliding = wallSliding;
}

void WallSlideAbility::reset()
{
    wallSliding = false;
}