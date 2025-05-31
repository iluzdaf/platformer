#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/player_state.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"

WallSlideAbility::WallSlideAbility(const WallSlideAbilityData &wallSlideAbilityData)
    : slideSpeed(wallSlideAbilityData.slideSpeed)
{
}

void WallSlideAbility::update(MovementContext &movementContext, float /*deltaTime*/)
{
    wallSliding = false;

    const glm::vec2 velocity = movementContext.getVelocity();
    if (movementContext.onGround() || velocity.y <= 0.0f)
    {
        return;
    }

    if (!(movementContext.touchingLeftWall() || movementContext.touchingRightWall()))
    {
        return;
    }

    wallSliding = true;
    glm::vec2 clampedVelocity = velocity;
    clampedVelocity.y = glm::min(velocity.y, slideSpeed);
    movementContext.setVelocity(clampedVelocity);
}

void WallSlideAbility::syncState(PlayerState &playerState) const
{
    playerState.wallSliding = wallSliding;
}