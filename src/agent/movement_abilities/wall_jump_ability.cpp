#include "agent/agent_state.hpp"
#include "agent/movement_abilities/wall_jump_ability.hpp"
#include "input/input_intentions.hpp"

WallJumpAbility::WallJumpAbility(const WallJumpAbilityData &data)
    : data(data),
      wallJumpBuffer(data.wallJumpBufferDuration),
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
    AgentState &state)
{
    state.emitWallJump = false;
    state.wallJumpVelocity = glm::vec2(0.0f);

    wallJumpBuffer.update(deltaTime);
    wallJumpCoyote.update(state.touchingLeftWall || state.touchingRightWall, deltaTime);

    if (state.onGround)
    {
        wallJumpCoyote.consume();
        return;
    }

    if (!state.wallJumping)
    {
        if (inputIntentions.jumpHeld)
        {
            wallJumpBuffer.press();
            wallJumpDirectionBuffer.press(inputIntentions.direction.x);
        }

        if (wallJumpBuffer.isBuffered())
        {
            int desiredDirection = 0;
            if (state.touchingLeftWall)
                desiredDirection = 1;
            else if (state.touchingRightWall)
                desiredDirection = -1;
            else
                desiredDirection = state.wasLastWallLeft ? 1 : -1;

            float bufferedDirection = wallJumpDirectionBuffer.getBufferedDirectionX();
            bool jumpInputCorrect = desiredDirection * bufferedDirection > 0;
            bool touchingWallNow = state.touchingLeftWall || state.touchingRightWall;
            if (touchingWallNow && jumpInputCorrect)
                startWallJump(state, desiredDirection);
            else if (!touchingWallNow && wallJumpCoyote.isCoyoteAvailable() && jumpInputCorrect)
                startWallJump(state, desiredDirection);
        }
    }

    if (state.wallJumping)
    {
        bool switchedSides =
            (state.touchingLeftWall && state.wallJumpDirection == -1) ||
            (state.touchingRightWall && state.wallJumpDirection == 1);

        if (switchedSides)
        {
            state.wallJumpTimeLeft = 0.0f;
            state.wallJumping = false;
            return;
        }

        state.wallJumpTimeLeft -= deltaTime;
        if (state.wallJumpTimeLeft <= 0.0f)
        {
            state.wallJumping = false;
            return;
        }

        state.wallJumpVelocity = {
            data.wallJumpHorizontalSpeed * state.wallJumpDirection,
            data.wallJumpSpeed};
    }
}

void WallJumpAbility::startWallJump(
    AgentState &state,
    int direction)
{
    state.wallJumpDirection = static_cast<float>(direction);
    state.wallJumpTimeLeft = data.wallJumpDuration;
    state.wallJumping = true;
    state.emitWallJump = true;
    wallJumpBuffer.consume();
    wallJumpDirectionBuffer.consume();
    wallJumpCoyote.consume();
}