#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/gravity_ability.hpp"
#include "input/input_intentions.hpp"
#include <algorithm>

GravityAbility::GravityAbility(const GravityAbilityData &data) : data(data)
{
}

void GravityAbility::decide(
    float deltaTime,
    const InputIntentions &,
    const Observed &observed,
    AbilityStates &states)
{
    if (observed.contacts.onGround || states.wallHang.active || states.wallSlide.active ||
        states.mantle.active || states.knockback.active)
        states.gravity.velocity.y = 0.0f;
    else
    {
        states.gravity.velocity.y += data.gravity * deltaTime;
        states.gravity.velocity.y = std::min(states.gravity.velocity.y, data.maxFallSpeed);
    }
}