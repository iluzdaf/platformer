

#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/player.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"

JumpAbility::JumpAbility(const JumpAbilityData &jumpAbilityData)
    : maxJumpCount(jumpAbilityData.maxJumpCount),
      jumpSpeed(jumpAbilityData.jumpSpeed)
{
    assert(maxJumpCount > 0);
    assert(jumpSpeed < 0);
}

void JumpAbility::fixedUpdate(
    MovementContext & /*movementContext*/,
    const PlayerState & /*playerState*/,
    float /*deltaTime*/)
{
}

void JumpAbility::update(
    MovementContext & /*movementContext*/,
    const PlayerState &playerState,
    float /*deltaTime*/)
{
    if (playerState.onGround)
    {
        resetJumps();
    }
}

void JumpAbility::tryJump(
    MovementContext &movementContext,
    const PlayerState &playerState)
{
    if (playerState.dashing || 
        playerState.wallSliding || 
        jumpCount >= maxJumpCount || 
        playerState.wallJumping)
        return;

    ++jumpCount;

    glm::vec2 velocity = movementContext.getVelocity();
    velocity.y = jumpSpeed;
    movementContext.setVelocity(velocity);
    if(jumpCount > 1)
        movementContext.emitDoubleJump();
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
}