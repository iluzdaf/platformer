#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/decided.hpp"
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
    Decided &decided)
{
    if (observed.contacts.onGround || decided.wallHang.active || decided.wallSlide.active ||
        decided.mantle.active || decided.knockback.active)
        decided.gravity.velocity.y = 0.0f;
    else
    {
        decided.gravity.velocity.y += data.gravity * deltaTime;
        decided.gravity.velocity.y = std::min(decided.gravity.velocity.y, data.maxFallSpeed);
    }
}