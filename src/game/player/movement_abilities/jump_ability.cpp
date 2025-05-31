

#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/player.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"

JumpAbility::JumpAbility(const JumpAbilityData &jumpAbilityData)
    : maxJumpCount(jumpAbilityData.maxJumpCount),
      jumpSpeed(jumpAbilityData.jumpSpeed)
{
}

void JumpAbility::update(MovementContext &movementContext, float deltaTime)
{
    if (movementContext.onGround())
    {
        resetJumps();
    }
}

void JumpAbility::tryJump(MovementContext &movementContext)
{
    if (movementContext.getPlayerState().dashing)
        return;

    if (jumpCount < maxJumpCount)
    {
        glm::vec2 velocity = movementContext.getVelocity();
        velocity.y = jumpSpeed;
        movementContext.setVelocity(velocity);
        jumpCount++;
        movementContext.setOnGround(false);
    }
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
}