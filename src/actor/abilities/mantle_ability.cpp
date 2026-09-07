#include <stdexcept>
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/mantle_ability.hpp"
#include "input/input_intentions.hpp"

MantleAbility::MantleAbility(const MantleAbilityData &data) : data(data)
{
    if (data.mantleSpeed <= 0)
        throw std::runtime_error("mantleSpeed must be greater than 0");
    if (data.mantleDuration <= 0)
        throw std::runtime_error("mantleDuration must be greater than 0");
}

void MantleAbility::applyMovement(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    Decided &decided)
{
    decided.mantle.velocity = glm::vec2(0.0f);

    if (!decided.mantle.active)
    {
        bool atLedge = observed.contacts.ledgeOnLeft || observed.contacts.ledgeOnRight;
        if (!decided.wallHang.active || !atLedge || inputIntentions.direction.y >= 0.0f)
            return;

        decided.mantle.direction = observed.contacts.ledgeOnLeft ? -1.0f : 1.0f;
        decided.mantle.timeLeft = data.mantleDuration;
        decided.mantle.active = true;
    }

    decided.mantle.timeLeft -= deltaTime;
    if (decided.mantle.timeLeft <= 0.0f)
    {
        decided.mantle.timeLeft = 0.0f;
        decided.mantle.active = false;
        return;
    }

    bool pullingUp = decided.mantle.timeLeft > data.mantleDuration * 0.5f;
    decided.mantle.velocity = pullingUp
                                  ? glm::vec2(0.0f, -data.mantleSpeed)
                                  : glm::vec2(data.mantleSpeed * decided.mantle.direction, 0.0f);
}
