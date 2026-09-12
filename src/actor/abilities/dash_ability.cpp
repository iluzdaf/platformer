#include <cstdlib>
#include <stdexcept>
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/dash_ability.hpp"
#include "input/input_intentions.hpp"

DashAbility::DashAbility(const DashAbilityData &data) : data(data)
{
    if (data.dashSpeed <= 0)
        throw std::runtime_error("A dash needs a speed above 0");
    if (data.dashDuration <= 0)
        throw std::runtime_error("A dash needs a duration above 0");
    if (data.airborneFraction <= 0 || data.airborneFraction > 1)
        throw std::runtime_error(
            "A dash in the air needs a fraction of its duration above 0 and at most 1");
}

void DashAbility::decide(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    AbilityStates &states)
{
    states.dash.emit = false;
    states.dash.velocity = glm::vec2(0.0f);

    if (observed.contacts.onGround && states.dash.timeLeft <= 0.0f)
        states.dash.available = true;

    if (inputIntentions.dashRequested && std::abs(inputIntentions.direction.x) > 0.0f &&
        states.dash.available && !observed.contacts.touchingLeftWall &&
        !observed.contacts.touchingRightWall)
    {
        states.dash.direction = inputIntentions.direction.x;
        states.dash.timeLeft = observed.contacts.onGround
                                   ? data.dashDuration
                                   : data.dashDuration * data.airborneFraction;
        states.dash.available = false;
        states.dash.emit = true;
        states.dash.active = true;
    }

    if (states.dash.timeLeft > 0.0f && states.dash.active)
    {
        if (observed.contacts.touchingWall())
        {
            states.dash.timeLeft = 0.0f;
            states.dash.active = false;
        }
        else
        {
            states.dash.timeLeft -= deltaTime;

            if (states.dash.timeLeft > 0.0f)
                states.dash.velocity.x = data.dashSpeed * states.dash.direction;
            else
            {
                states.dash.timeLeft = 0.0f;
                states.dash.active = false;
            }
        }
    }
    else if (states.dash.timeLeft <= 0.0f)
        states.dash.active = false;
}