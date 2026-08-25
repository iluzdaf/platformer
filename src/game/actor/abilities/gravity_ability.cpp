#include "game/actor/actor_motion_state.hpp"
#include "game/actor/abilities/gravity_ability.hpp"
#include "input/input_intentions.hpp"

GravityAbility::GravityAbility(const GravityAbilityData &data)
    : data(data)
{
}

void GravityAbility::applyMovement(
    float,
    const InputIntentions &,
    ActorMotionState &state)
{
    if (state.onGround || state.climbing || state.wallSliding)
        state.gravityVelocity.y = 0.0f;
    else
    {
        state.gravityVelocity.y += data.gravity;
        state.gravityVelocity.y = std::min(state.gravityVelocity.y, data.maxFallSpeed);
    }
}