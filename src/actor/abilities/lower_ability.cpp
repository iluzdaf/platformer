#include <stdexcept>
#include "actor/abilities/lower_ability_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/lower_ability.hpp"
#include "input/input_intentions.hpp"

LowerAbility::LowerAbility(const LowerAbilityData &data) : data(data)
{
    if (data.lowerSpeed <= 0)
        throw std::runtime_error("A lower needs a speed above 0");
    if (data.lowerDuration <= 0)
        throw std::runtime_error("A lower needs a duration above 0");
}

void LowerAbility::decide(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    AbilityStates &states)
{
    states.lower.velocity = glm::vec2(0.0f);
    const ActorContactState &contacts = observed.contacts;

    if (!states.lower.active)
    {
        bool atAnEdge = contacts.edgeOnLeft || contacts.edgeOnRight;
        if (!contacts.onGround || !atAnEdge || !inputIntentions.climbRequested ||
            inputIntentions.direction.y <= 0.0f)
            return;

        states.lower.direction = contacts.edgeOnLeft ? -1.0f : 1.0f;
        states.lower.timeLeft = data.lowerDuration;
        states.lower.dropping = false;
        states.lower.active = true;
    }

    if (!contacts.onGround)
        states.lower.dropping = true;

    bool againstTheWall = states.lower.direction < 0.0f
                              ? contacts.grippableRightWall && !contacts.touchingLeftWall
                              : contacts.grippableLeftWall && !contacts.touchingRightWall;
    states.lower.timeLeft -= deltaTime;
    if (states.lower.timeLeft <= 0.0f || (states.lower.dropping && againstTheWall))
    {
        states.lower.timeLeft = 0.0f;
        states.lower.active = false;
        return;
    }

    states.lower.velocity = states.lower.dropping
                                ? glm::vec2(0.0f, data.lowerSpeed)
                                : glm::vec2(states.lower.direction * data.lowerSpeed, 0.0f);
}
