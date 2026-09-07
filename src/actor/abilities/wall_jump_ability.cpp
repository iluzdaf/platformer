#include <stdexcept>
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/wall_jump_ability.hpp"
#include "input/input_intentions.hpp"

WallJumpAbility::WallJumpAbility(const WallJumpAbilityData &data)
    : data(data), wallJumpBuffer(data.wallJumpBufferDuration),
      wallJumpCoyote(data.wallJumpCoyoteDuration)
{
    if (data.wallJumpSpeed >= 0)
        throw std::runtime_error("wallJumpSpeed must be negative");
    if (data.wallJumpHorizontalSpeed <= 0)
        throw std::runtime_error("wallJumpHorizontalSpeed must be greater than 0");
}

void WallJumpAbility::applyMovement(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    Decided &decided)
{
    decided.wallJump.emit = false;
    decided.wallJump.velocity = glm::vec2(0.0f);

    wallJumpBuffer.update(deltaTime);
    wallJumpCoyote.update(observed.contacts.grippableWall(), deltaTime);

    if (observed.contacts.onGround)
    {
        wallJumpCoyote.consume();
        return;
    }

    if (!decided.wallJump.active)
    {
        if (inputIntentions.jumpHeld)
        {
            wallJumpBuffer.press();
            wallJumpDirectionBuffer.press(inputIntentions.direction.x);
        }

        if (wallJumpBuffer.isBuffered())
        {
            int desiredDirection = 0;
            if (observed.contacts.grippableLeftWall)
                desiredDirection = 1;
            else if (observed.contacts.grippableRightWall)
                desiredDirection = -1;
            else
                desiredDirection = observed.contacts.wasLastWallLeft ? 1 : -1;

            float bufferedDirection = wallJumpDirectionBuffer.getBufferedDirectionX();
            bool jumpInputCorrect = desiredDirection * bufferedDirection > 0;
            bool grippableWallNow = observed.contacts.grippableWall();
            if (jumpInputCorrect && (grippableWallNow || wallJumpCoyote.isCoyoteAvailable()))
                startWallJump(decided, desiredDirection);
        }
    }

    if (decided.wallJump.active)
    {
        bool switchedSides =
            (observed.contacts.touchingLeftWall && decided.wallJump.direction == -1) ||
            (observed.contacts.touchingRightWall && decided.wallJump.direction == 1);

        if (switchedSides)
        {
            decided.wallJump.timeLeft = 0.0f;
            decided.wallJump.active = false;
            return;
        }

        decided.wallJump.timeLeft -= deltaTime;
        if (decided.wallJump.timeLeft <= 0.0f)
        {
            decided.wallJump.active = false;
            return;
        }

        decided.wallJump.velocity = {
            data.wallJumpHorizontalSpeed * decided.wallJump.direction, data.wallJumpSpeed};
    }
}

void WallJumpAbility::startWallJump(Decided &decided, int direction)
{
    decided.wallJump.direction = static_cast<float>(direction);
    decided.wallJump.timeLeft = data.wallJumpDuration;
    decided.wallJump.active = true;
    decided.wallJump.emit = true;
    wallJumpBuffer.consume();
    wallJumpDirectionBuffer.consume();
    wallJumpCoyote.consume();
}