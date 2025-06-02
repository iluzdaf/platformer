#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/player_state.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"

WallSlideAbility::WallSlideAbility(const WallSlideAbilityData &wallSlideAbilityData)
    : slideSpeed(wallSlideAbilityData.slideSpeed)
{
    assert(slideSpeed > 0);
}

void WallSlideAbility::fixedUpdate(
    MovementContext &movementContext,
    const PlayerState &playerState,
    float /*deltaTime*/)
{
    wallSliding = false;

    if (playerState.onGround)
    {
        return;
    }

    if (!(playerState.touchingLeftWall || playerState.touchingRightWall))
    {
        return;
    }

    wallSliding = true;
    
    const glm::vec2 velocity = movementContext.getVelocity();
    glm::vec2 clampedVelocity = velocity;
    clampedVelocity.y = glm::min(velocity.y, slideSpeed);
    movementContext.setVelocity(clampedVelocity);
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