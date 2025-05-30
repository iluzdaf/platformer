

#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/player.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"

JumpAbility::JumpAbility(const JumpAbilityData &jumpAbilityData)
    : maxJumpCount(jumpAbilityData.maxJumpCount),
      jumpSpeed(jumpAbilityData.jumpSpeed)
{
}

void JumpAbility::update(MovementContext &context, float deltaTime)
{
    if (context.onGround())
    {
        resetJumps();
    }
}

void JumpAbility::tryJump(MovementContext &context)
{
    DashAbility *dash = context.getAbility<DashAbility>();
    if (dash && dash->dashing())
        return;

    if (jumpCount < maxJumpCount)
    {
        glm::vec2 velocity = context.getVelocity();
        velocity.y = jumpSpeed;
        context.setVelocity(velocity);
        jumpCount++;
        context.setOnGround(false);
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