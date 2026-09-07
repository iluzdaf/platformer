#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/gravity_ability.hpp"
#include "input/input_intentions.hpp"
#include <algorithm>

GravityAbility::GravityAbility(const GravityAbilityData &data) : data(data)
{
}

void GravityAbility::applyMovement(
    float deltaTime,
    const InputIntentions &,
    const Observed &observed,
    Decided &state)
{
    if (observed.contacts.onGround || state.wallHang.active || state.wallSlide.active ||
        state.mantle.active || state.knockback.active)
        state.gravity.velocity.y = 0.0f;
    else
    {
        state.gravity.velocity.y += data.gravity * deltaTime;
        state.gravity.velocity.y = std::min(state.gravity.velocity.y, data.maxFallSpeed);
    }
}