#include <stdexcept>
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
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
    ActorMotionState &state)
{
    state.wallJump.emit = false;
    state.wallJump.velocity = glm::vec2(0.0f);

    wallJumpBuffer.update(deltaTime);
    wallJumpCoyote.update(
        state.contacts.touchingLeftWall || state.contacts.touchingRightWall, deltaTime);

    if (state.contacts.onGround)
    {
        wallJumpCoyote.consume();
        return;
    }

    if (!state.wallJump.active)
    {
        if (inputIntentions.jumpHeld)
        {
            wallJumpBuffer.press();
            wallJumpDirectionBuffer.press(inputIntentions.direction.x);
        }

        if (wallJumpBuffer.isBuffered())
        {
            int desiredDirection = 0;
            if (state.contacts.touchingLeftWall)
                desiredDirection = 1;
            else if (state.contacts.touchingRightWall)
                desiredDirection = -1;
            else
                desiredDirection = state.contacts.wasLastWallLeft ? 1 : -1;

            float bufferedDirection = wallJumpDirectionBuffer.getBufferedDirectionX();
            bool jumpInputCorrect = desiredDirection * bufferedDirection > 0;
            bool touchingWallNow =
                state.contacts.touchingLeftWall || state.contacts.touchingRightWall;
            if (touchingWallNow && jumpInputCorrect)
                startWallJump(state, desiredDirection);
            else if (!touchingWallNow && wallJumpCoyote.isCoyoteAvailable() && jumpInputCorrect)
                startWallJump(state, desiredDirection);
        }
    }

    if (state.wallJump.active)
    {
        bool switchedSides = (state.contacts.touchingLeftWall && state.wallJump.direction == -1) ||
                             (state.contacts.touchingRightWall && state.wallJump.direction == 1);

        if (switchedSides)
        {
            state.wallJump.timeLeft = 0.0f;
            state.wallJump.active = false;
            return;
        }

        state.wallJump.timeLeft -= deltaTime;
        if (state.wallJump.timeLeft <= 0.0f)
        {
            state.wallJump.active = false;
            return;
        }

        state.wallJump.velocity = {
            data.wallJumpHorizontalSpeed * state.wallJump.direction, data.wallJumpSpeed};
    }
}

void WallJumpAbility::startWallJump(ActorMotionState &state, int direction)
{
    state.wallJump.direction = static_cast<float>(direction);
    state.wallJump.timeLeft = data.wallJumpDuration;
    state.wallJump.active = true;
    state.wallJump.emit = true;
    wallJumpBuffer.consume();
    wallJumpDirectionBuffer.consume();
    wallJumpCoyote.consume();
}