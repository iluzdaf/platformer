#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/player.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"

JumpAbility::JumpAbility(const JumpAbilityData &jumpAbilityData)
    : maxJumpCount(jumpAbilityData.maxJumpCount),
      jumpSpeed(jumpAbilityData.jumpSpeed),
      jumpBufferDuration(jumpAbilityData.jumpBufferDuration)
{
    if (maxJumpCount <= 0)
        throw std::invalid_argument("maxJumpCount must be greater than 0");
    if (jumpSpeed >= 0)
        throw std::invalid_argument("jumpSpeed must be negative");
}

void JumpAbility::fixedUpdate(
    MovementContext &movementContext,
    const PlayerState &playerState,
    float deltaTime)
{
    if (jumpBufferTime > 0.0f)
        jumpBufferTime -= deltaTime;

    if (playerState.onGround && jumpBufferTime > 0.0f)
    {
        performJump(movementContext);
        jumpBufferTime = 0.0f;
    }
}

void JumpAbility::update(
    MovementContext & /*movementContext*/,
    const PlayerState &playerState,
    float /*deltaTime*/)
{
    if (playerState.onGround)
        resetJumps();
}

void JumpAbility::tryJump(
    MovementContext &movementContext,
    const PlayerState &playerState)
{
    if (playerState.dashing ||
        playerState.wallSliding ||
        playerState.wallJumping)
        return;

    if ((!playerState.onGround && (playerState.touchingLeftWall || playerState.touchingRightWall)))
        return;

    if (jumpCount >= maxJumpCount)
        return;

    if (!playerState.onGround && jumpCount == 0)
    {
        jumpBufferTime = jumpBufferDuration;
        return;
    }

    performJump(movementContext);
}

void JumpAbility::resetJumps()
{
    jumpCount = 0;
}

int JumpAbility::getMaxJumpCount() const
{
    return maxJumpCount;
}

float JumpAbility::getJumpSpeed() const
{
    return jumpSpeed;
}

void JumpAbility::syncState(PlayerState &playerState) const
{
    playerState.jumpSpeed = jumpSpeed;
    playerState.maxJumpCount = maxJumpCount;
    playerState.jumpCount = jumpCount;
}

void JumpAbility::reset()
{
    resetJumps();
    jumpBufferTime = 0;
}

void JumpAbility::performJump(MovementContext &movementContext)
{
    ++jumpCount;

    glm::vec2 velocity = movementContext.getVelocity();
    velocity.y = jumpSpeed;
    movementContext.setVelocity(velocity);
    if (jumpCount > 1)
        movementContext.emitDoubleJump();
}