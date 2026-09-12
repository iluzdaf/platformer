#include <stdexcept>
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/jump_ability.hpp"
#include "input/input_intentions.hpp"

JumpAbility::JumpAbility(const JumpAbilityData &data)
    : data(data), jumpBuffer(data.jumpBufferDuration, "A jump's buffer"),
      coyoteTime(data.jumpCoyoteDuration, "A jump's leeway off a ledge")
{
    if (data.jumpSpeed >= 0)
        throw std::runtime_error("A jump needs a speed upward, below 0");
}

void JumpAbility::decide(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    AbilityStates &states)
{
    states.jump.velocity = glm::vec2(0.0f);

    jumpBuffer.update(deltaTime);
    coyoteTime.update(deltaTime);
    if (observed.contacts.onGround)
        coyoteTime.start();

    if (!states.jump.active)
    {
        if (inputIntentions.jumpRequested)
            jumpBuffer.start();

        if (jumpBuffer.running() && (observed.contacts.onGround || coyoteTime.running()))
        {
            states.jump.active = true;
            states.jump.holdTime = 0.0f;
            jumpBuffer.consume();
            coyoteTime.consume();
        }
    }

    if (states.jump.active)
    {
        states.jump.holdTime += deltaTime;

        bool stillGoingUp = states.jump.holdTime <= data.jumpDuration &&
                            (inputIntentions.jumpHeld || inputIntentions.jumpRequested) &&
                            !observed.contacts.hitCeiling;

        if (stillGoingUp)
            states.jump.velocity.y = data.jumpSpeed;
        else
            states.jump.active = false;
    }
}