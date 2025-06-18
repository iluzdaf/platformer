#include "game/player/movement_abilities/wall_jump_ability.hpp"
#include "game/player/movement_abilities/wall_jump_ability_data.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/player_state.hpp"

WallJumpAbility::WallJumpAbility(const WallJumpAbilityData &wallJumpAbilityData)
    : jumpSpeed(wallJumpAbilityData.jumpSpeed),
      horizontalSpeed(wallJumpAbilityData.horizontalSpeed),
      maxJumpCount(wallJumpAbilityData.maxJumpCount),
      wallJumpDuration(wallJumpAbilityData.wallJumpDuration),
      wallJumpBufferDuration(wallJumpAbilityData.wallJumpBufferDuration)
{
    if (jumpSpeed >= 0)
        throw std::invalid_argument("jumpSpeed must be negative");
    if (horizontalSpeed <= 0)
        throw std::invalid_argument("horizontalSpeed must be greater than 0");
    if (maxJumpCount <= 0)
        throw std::invalid_argument("maxJumpCount must be greater than 0");
}

void WallJumpAbility::fixedUpdate(
    MovementContext &movementContext,
    const PlayerState &playerState,
    float deltaTime)
{
    if (wallJumpBufferTime > 0.0f)
        wallJumpBufferTime -= deltaTime;

    if ((playerState.touchingLeftWall || playerState.touchingRightWall) &&
        wallJumpBufferTime > 0.0f)
    {
        performWallJump(movementContext, playerState);
        wallJumpBufferTime = 0.0f;
    }

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
    const PlayerState &playerState,
    float /*deltaTime*/)
{
    if (playerState.onGround)
        resetJumps();
}

void WallJumpAbility::tryJump(
    MovementContext &movementContext,
    const PlayerState &playerState)
{
    if (playerState.onGround || jumpCount >= maxJumpCount)
        return;

    if (playerState.touchingLeftWall || playerState.touchingRightWall)
        performWallJump(movementContext, playerState);
    else
        wallJumpBufferTime = wallJumpBufferDuration;
}

void WallJumpAbility::performWallJump(
    MovementContext &movementContext,
    const PlayerState &playerState)
{
    wasTouchingLeftWall = playerState.touchingLeftWall;
    wallJumpDirection = wasTouchingLeftWall ? 1 : -1;
    wallJumpTimeLeft = wallJumpDuration;
    ++jumpCount;
    glm::vec2 velocity = movementContext.getVelocity();
    velocity.y = jumpSpeed;
    movementContext.setVelocity(velocity);
    movementContext.setFacingLeft(!wasTouchingLeftWall);
    movementContext.emitWallJump();
}

void WallJumpAbility::syncState(PlayerState &playerState) const
{
    playerState.wallJumping = wallJumping();
}

bool WallJumpAbility::wallJumping() const
{
    return wallJumpTimeLeft > 0.0f;
}

void WallJumpAbility::resetJumps()
{
    jumpCount = 0;
}

void WallJumpAbility::reset()
{
    resetJumps();
    wallJumpTimeLeft = 0;
    wasTouchingLeftWall = false;
    wallJumpDirection = 1;
    wallJumpBufferTime = 0;
}