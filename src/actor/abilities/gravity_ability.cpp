#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/gravity_ability.hpp"
#include "input/input_intentions.hpp"
#include <algorithm>

GravityAbility::GravityAbility(const GravityAbilityData &data) : data(data)
{
}

void GravityAbility::applyMovement(
    float deltaTime,
    const InputIntentions &,
    ActorMotionState &state)
{
    if (state.contacts.onGround || state.climb.active || state.wallSlide.active)
        state.gravity.velocity.y = 0.0f;
    else
    {
        state.gravity.velocity.y += data.gravity * deltaTime;
        state.gravity.velocity.y = std::min(state.gravity.velocity.y, data.maxFallSpeed);
    }
}