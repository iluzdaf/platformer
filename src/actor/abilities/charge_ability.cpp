#include <stdexcept>
#include "actor/abilities/charge_ability.hpp"
#include "actor/abilities/charge_ability_data.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

namespace
{
    bool stoppedAhead(const ActorContactState &contacts, float direction)
    {
        if (direction < 0.0f)
            return contacts.touchingLeftWall || contacts.touchingLeftEdge;

        return contacts.touchingRightWall || contacts.touchingRightEdge;
    }
}

ChargeAbility::ChargeAbility(const ChargeAbilityData &data) : data(data)
{
    if (data.speed <= 0.0f)
        throw std::runtime_error("A charge needs speed above 0");

    if (data.damage <= 0)
        throw std::runtime_error("A charge needs damage above 0");
}

void ChargeAbility::decide(
    float,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    AbilityStates &states)
{
    ChargeAbilityState &charge = states.charge;
    charge.emit = false;

    if (states.knockback.active)
    {
        charge.active = false;
        return;
    }

    if (charge.active)
    {
        if (stoppedAhead(observed.contacts, charge.direction))
        {
            charge.active = false;
            return;
        }

        charge.velocity = glm::vec2(data.speed * charge.direction, states.gravity.velocity.y);
        return;
    }

    if (inputIntentions.attack != ChargeAttack || !observed.contacts.onGround)
        return;

    if (inputIntentions.direction.x != 0.0f)
        charge.direction = inputIntentions.direction.x < 0.0f ? -1.0f : 1.0f;
    else
        charge.direction = observed.facingLeft ? -1.0f : 1.0f;

    if (stoppedAhead(observed.contacts, charge.direction))
        return;

    charge.active = true;
    charge.emit = true;
    charge.damage = data.damage;
    charge.velocity = glm::vec2(data.speed * charge.direction, states.gravity.velocity.y);
}
