#include <GLFW/glfw3.h>
#include "game/player/movement_abilities/jump_ability.hpp"
#include "game/player/player.hpp"

JumpAbility::JumpAbility(int maxJumps, float jumpVelocity)
    : maxJumps(maxJumps), remainingJumps(maxJumps), jumpVelocity(jumpVelocity)
{
}

void JumpAbility::update(Player &player, float /*deltaTime*/)
{
    bool isPressed = glfwGetKey(player.getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS;

    if (isPressed && !wasJumpPressed && remainingJumps > 0)
    {
        player.setVelocityY(jumpVelocity);
        onJump();
    }

    wasJumpPressed = isPressed;
}

void JumpAbility::resetJumps()
{
    remainingJumps = maxJumps;
}

void JumpAbility::onJump()
{
    --remainingJumps;
}

int JumpAbility::getRemainingJumps() const
{
    return remainingJumps;
}