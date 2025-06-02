#include "game/player/movement_abilities/wall_jump_ability.hpp"
#include "game/player/movement_abilities/wall_jump_ability_data.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/player_state.hpp"

WallJumpAbility::WallJumpAbility(const WallJumpAbilityData &wallJumpAbilityData)
    : jumpSpeed(wallJumpAbilityData.jumpSpeed),
      horizontalSpeed(wallJumpAbilityData.horizontalSpeed)
{
    assert(jumpSpeed < 0);
    assert(horizontalSpeed > 0);
}

void WallJumpAbility::fixedUpdate(
    MovementContext &movementContext,
    const PlayerState &playerState,
    float deltaTime)
{
    if (!wallJumping())
        return;

    bool wallSwitchedSides =
        (playerState.touchingLeftWall && !wasTouchingLeftWall) ||
        (playerState.touchingRightWall && wasTouchingLeftWall);

    if (wallSwitchedSides)
    {
        wallJumpTimeLeft = 0;
        movementContext.setVelocity({0.0f, 0.0f});
        return;
    }

    wallJumpTimeLeft -= deltaTime;

    glm::vec2 velocity = movementContext.getVelocity();
    velocity.x = horizontalSpeed * wallJumpDirection;
    movementContext.setVelocity(velocity);
}

void WallJumpAbility::update(
    MovementContext & /*movementContext*/,
    const PlayerState & /*playerState*/,
    float /*deltaTime*/)
{
}

void WallJumpAbility::tryJump(
    MovementContext &movementContext,
    const PlayerState &playerState)
{
    if (!playerState.wallSliding)
        return;

    wasTouchingLeftWall = playerState.touchingLeftWall;
    wallJumpDirection = wasTouchingLeftWall ? 1 : -1;
    wallJumpTimeLeft = wallJumpDuration;

    glm::vec2 velocity = movementContext.getVelocity();
    velocity.y = jumpSpeed;
    movementContext.setVelocity(velocity);
}

void WallJumpAbility::syncState(PlayerState &playerState) const
{
    playerState.wallJumping = wallJumping();
}

bool WallJumpAbility::wallJumping() const
{
    return wallJumpTimeLeft > 0.0f;
}