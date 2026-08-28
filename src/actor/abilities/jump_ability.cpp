#include <stdexcept>
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/jump_ability.hpp"
#include "input/input_intentions.hpp"

JumpAbility::JumpAbility(const JumpAbilityData &data)
    : data(data), jumpBuffer(data.jumpBufferDuration), coyoteTime(data.jumpCoyoteDuration)
{
    if (data.jumpSpeed >= 0)
        throw std::runtime_error("jumpSpeed must be negative");
}

void JumpAbility::applyMovement(
    float deltaTime,
    const InputIntentions &inputIntentions,
    ActorMotionState &state)
{
    state.jump.velocity = glm::vec2(0.0f);

    jumpBuffer.update(deltaTime);
    coyoteTime.update(state.contacts.onGround, deltaTime);

    if (!state.jump.active)
    {
        if (inputIntentions.jumpRequested)
            jumpBuffer.press();

        if (jumpBuffer.isBuffered() && (state.contacts.onGround || coyoteTime.isCoyoteAvailable()))
        {
            state.jump.active = true;
            state.jump.holdTime = 0.0f;
            jumpBuffer.consume();
            coyoteTime.consume();
        }
    }

    if (state.jump.active)
    {
        state.jump.holdTime += deltaTime;

        bool stillGoingUp = state.jump.holdTime <= data.jumpDuration &&
                            (inputIntentions.jumpHeld || inputIntentions.jumpRequested) &&
                            !state.contacts.hitCeiling;

        if (stillGoingUp)
            state.jump.velocity.y = data.jumpSpeed;
        else
            state.jump.active = false;
    }
}