#include <stdexcept>
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
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
    ActorMotionState &state)
{
    state.mantle.velocity = glm::vec2(0.0f);

    if (!state.mantle.active)
    {
        bool atLedge = state.contacts.ledgeOnLeft || state.contacts.ledgeOnRight;
        if (!state.wallHang.active || !atLedge || inputIntentions.direction.y >= 0.0f)
            return;

        state.mantle.direction = state.contacts.ledgeOnLeft ? -1.0f : 1.0f;
        state.mantle.timeLeft = data.mantleDuration;
        state.mantle.active = true;
    }

    state.mantle.timeLeft -= deltaTime;
    if (state.mantle.timeLeft <= 0.0f)
    {
        state.mantle.timeLeft = 0.0f;
        state.mantle.active = false;
        return;
    }

    bool pullingUp = state.mantle.timeLeft > data.mantleDuration * 0.5f;
    state.mantle.velocity = pullingUp ? glm::vec2(0.0f, -data.mantleSpeed)
                                      : glm::vec2(data.mantleSpeed * state.mantle.direction, 0.0f);
}
