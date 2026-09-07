#include <stdexcept>
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
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
    const Observed &observed,
    Decided &decided)
{
    decided.jump.velocity = glm::vec2(0.0f);

    jumpBuffer.update(deltaTime);
    coyoteTime.update(observed.contacts.onGround, deltaTime);

    if (!decided.jump.active)
    {
        if (inputIntentions.jumpRequested)
            jumpBuffer.press();

        if (jumpBuffer.isBuffered() &&
            (observed.contacts.onGround || coyoteTime.isCoyoteAvailable()))
        {
            decided.jump.active = true;
            decided.jump.holdTime = 0.0f;
            jumpBuffer.consume();
            coyoteTime.consume();
        }
    }

    if (decided.jump.active)
    {
        decided.jump.holdTime += deltaTime;

        bool stillGoingUp = decided.jump.holdTime <= data.jumpDuration &&
                            (inputIntentions.jumpHeld || inputIntentions.jumpRequested) &&
                            !observed.contacts.hitCeiling;

        if (stillGoingUp)
            decided.jump.velocity.y = data.jumpSpeed;
        else
            decided.jump.active = false;
    }
}