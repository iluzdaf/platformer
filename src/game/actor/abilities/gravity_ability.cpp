#include "game/actor/actor_motion_state.hpp"
#include "game/actor/abilities/gravity_ability.hpp"
#include "input/input_intentions.hpp"

GravityAbility::GravityAbility(const GravityAbilityData &data)
    : data(data)
{
}

void GravityAbility::applyMovement(
    float deltaTime,
    const InputIntentions &,
    ActorMotionState &state)
{
    if (state.contacts.onGround || state.climbing || state.wallSliding)
        state.gravityVelocity.y = 0.0f;
    else
    {
        state.gravityVelocity.y += data.gravity * deltaTime;
        state.gravityVelocity.y = std::min(state.gravityVelocity.y, data.maxFallSpeed);
    }
}