#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/knockback_ability.hpp"
#include "actor/abilities/knockback_ability_data.hpp"
#include "actor/abilities/knockback_ability_state.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/hit.hpp"
#include "actor/observed.hpp"

KnockbackAbility::KnockbackAbility(const KnockbackAbilityData &data) : data(data)
{
    if (data.speed <= 0.0f)
        throw std::runtime_error("A knockback needs a speed above 0");

    if (data.lift > 0.0f)
        throw std::runtime_error("A knockback lifts upward, so its lift is 0 or negative");

    if (data.duration <= 0.0f)
        throw std::runtime_error("A knockback needs a duration above 0");
}

void KnockbackAbility::applyMovement(
    float deltaTime,
    const InputIntentions &,
    const Observed &observed,
    ActorMotionState &state)
{
    KnockbackAbilityState &knockback = state.knockback;
    knockback.emit = false;
    knockback.velocity = glm::vec2(0.0f);

    if (!observed.hits.empty())
    {
        const Hit &last = observed.hits.back();
        if (last.direction.x != 0.0f)
            knockback.direction = last.direction.x < 0.0f ? -1.0f : 1.0f;

        knockback.timeLeft = data.duration;
        knockback.active = true;
        knockback.emit = true;
    }

    if (!knockback.active)
        return;

    knockback.timeLeft -= deltaTime;
    if (knockback.timeLeft <= 0.0f)
    {
        knockback.timeLeft = 0.0f;
        knockback.active = false;
        return;
    }

    knockback.velocity = glm::vec2(data.speed * knockback.direction, data.lift);
}
