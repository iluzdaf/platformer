#include <stdexcept>
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/mantle_ability.hpp"
#include "input/input_intentions.hpp"

MantleAbility::MantleAbility(const MantleAbilityData &data) : data(data)
{
    if (data.mantleSpeed <= 0)
        throw std::runtime_error("A mantle needs a speed above 0");
    if (data.mantleDuration <= 0)
        throw std::runtime_error("A mantle needs a duration above 0");
}

void MantleAbility::decide(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    AbilityStates &states)
{
    states.mantle.velocity = glm::vec2(0.0f);

    if (!states.mantle.active)
    {
        bool atLedge = observed.contacts.ledgeOnLeft || observed.contacts.ledgeOnRight;
        if (!states.wallHang.active || !atLedge || inputIntentions.direction.y >= 0.0f)
            return;

        states.mantle.direction = observed.contacts.ledgeOnLeft ? -1.0f : 1.0f;
        states.mantle.timeLeft = data.mantleDuration;
        states.mantle.active = true;
    }

    states.mantle.timeLeft -= deltaTime;
    if (states.mantle.timeLeft <= 0.0f)
    {
        states.mantle.timeLeft = 0.0f;
        states.mantle.active = false;
        return;
    }

    bool pullingUp = states.mantle.timeLeft > data.mantleDuration * 0.5f;
    states.mantle.velocity = pullingUp
                                 ? glm::vec2(0.0f, -data.mantleSpeed)
                                 : glm::vec2(data.mantleSpeed * states.mantle.direction, 0.0f);
}
