#include <stdexcept>
#include "actor/abilities/pounce_ability.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

PounceAbility::PounceAbility(const PounceAbilityData &data) : data(data)
{
    if (data.leap.x <= 0.0f || data.leap.y >= 0.0f)
        throw std::runtime_error("A pounce needs a leap forward and upward");

    if (data.damage <= 0)
        throw std::runtime_error("A pounce needs damage above 0");
}

void PounceAbility::decide(
    float,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    AbilityStates &states)
{
    PounceAbilityState &pounce = states.pounce;
    pounce.emit = false;

    if (states.knockback.active)
    {
        pounce.active = false;
        return;
    }

    if (pounce.active)
    {
        if (!observed.contacts.onGround)
            pounce.leftTheGround = true;
        else if (pounce.leftTheGround)
        {
            pounce.active = false;
            return;
        }

        pounce.velocity =
            glm::vec2(data.leap.x * pounce.direction, data.leap.y + states.gravity.velocity.y);
        return;
    }

    if (inputIntentions.attack != PounceAttack || !observed.contacts.onGround)
        return;

    pounce.active = true;
    pounce.emit = true;
    pounce.leftTheGround = false;
    if (inputIntentions.direction.x != 0.0f)
        pounce.direction = inputIntentions.direction.x < 0.0f ? -1.0f : 1.0f;
    else
        pounce.direction = observed.facingLeft ? -1.0f : 1.0f;
    pounce.damage = data.damage;
    pounce.velocity = glm::vec2(data.leap.x * pounce.direction, data.leap.y);
}
