#include <stdexcept>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/jump_ability.hpp"
#include "input/input_intentions.hpp"

JumpAbility::JumpAbility(const JumpAbilityData &data)
    : data(data),
      jumpBuffer(data.jumpBufferDuration),
      coyoteTime(data.jumpCoyoteDuration)
{
    if (data.jumpSpeed >= 0)
        throw std::runtime_error("jumpSpeed must be negative");
}

void JumpAbility::applyMovement(
    float deltaTime,
    const InputIntentions &inputIntentions,
    AgentState &state)
{
    state.jumpVelocity = glm::vec2(0.0f);
    
    jumpBuffer.update(deltaTime);
    coyoteTime.update(state.onGround, deltaTime);

    if (!state.jumping)
    {
        if (inputIntentions.jumpRequested)
            jumpBuffer.press();

        if (jumpBuffer.isBuffered() &&
            (state.onGround || coyoteTime.isCoyoteAvailable()))
        {
            state.jumping = true;
            state.jumpHoldTime = 0.0f;
            jumpBuffer.consume();
            coyoteTime.consume();
        }
    }

    if (state.jumping)
    {
        state.jumpHoldTime += deltaTime;
        if (state.jumpHoldTime <= data.jumpDuration &&
            (inputIntentions.jumpHeld || inputIntentions.jumpRequested))
            state.jumpVelocity.y = data.jumpSpeed;
        else
            state.jumping = false;
    }
}