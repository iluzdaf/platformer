#include <stdexcept>
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/wall_jump_ability.hpp"
#include "input/input_intentions.hpp"

WallJumpAbility::WallJumpAbility(const WallJumpAbilityData &data)
    : data(data), wallJumpBuffer(data.wallJumpBufferDuration, "A wall jump's buffer"),
      wallJumpCoyote(data.wallJumpCoyoteDuration, "A wall jump's leeway off a wall")
{
    if (data.wallJumpSpeed >= 0)
        throw std::runtime_error("A wall jump needs a speed upward, below 0");
    if (data.wallJumpHorizontalSpeed <= 0)
        throw std::runtime_error("A wall jump needs a speed away from the wall above 0");
}

void WallJumpAbility::decide(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    AbilityStates &states)
{
    states.wallJump.emit = false;
    states.wallJump.velocity = glm::vec2(0.0f);

    wallJumpBuffer.update(deltaTime);
    wallJumpCoyote.update(deltaTime);
    if (observed.contacts.grippableWall())
        wallJumpCoyote.start();

    if (observed.contacts.onGround)
    {
        wallJumpCoyote.consume();
        return;
    }

    if (!states.wallJump.active)
    {
        if (inputIntentions.jumpHeld)
            wallJumpBuffer.start(inputIntentions.direction.x);

        if (wallJumpBuffer.running())
        {
            int desiredDirection = 0;
            if (observed.contacts.grippableLeftWall)
                desiredDirection = 1;
            else if (observed.contacts.grippableRightWall)
                desiredDirection = -1;
            else
                desiredDirection = observed.contacts.wasLastWallLeft ? 1 : -1;

            float bufferedDirection = wallJumpBuffer.direction();
            bool jumpInputCorrect = desiredDirection * bufferedDirection > 0;
            bool grippableWallNow = observed.contacts.grippableWall();
            if (jumpInputCorrect && (grippableWallNow || wallJumpCoyote.running()))
                startWallJump(states, desiredDirection);
        }
    }

    if (states.wallJump.active)
    {
        bool switchedSides =
            (observed.contacts.touchingLeftWall && states.wallJump.direction == -1) ||
            (observed.contacts.touchingRightWall && states.wallJump.direction == 1);

        if (switchedSides)
        {
            states.wallJump.timeLeft = 0.0f;
            states.wallJump.active = false;
            return;
        }

        states.wallJump.timeLeft -= deltaTime;
        if (states.wallJump.timeLeft <= 0.0f)
        {
            states.wallJump.active = false;
            return;
        }

        states.wallJump.velocity = {
            data.wallJumpHorizontalSpeed * states.wallJump.direction, data.wallJumpSpeed};
    }
}

void WallJumpAbility::startWallJump(AbilityStates &states, int direction)
{
    states.wallJump.direction = static_cast<float>(direction);
    states.wallJump.timeLeft = data.wallJumpDuration;
    states.wallJump.active = true;
    states.wallJump.emit = true;
    wallJumpBuffer.consume();
    wallJumpCoyote.consume();
}