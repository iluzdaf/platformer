#include <cstdlib>
#include <stdexcept>
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/dash_ability.hpp"
#include "input/input_intentions.hpp"

DashAbility::DashAbility(const DashAbilityData &data) : data(data)
{
    if (data.dashSpeed <= 0)
        throw std::runtime_error("dashSpeed must be > 0");
    if (data.dashDuration <= 0)
        throw std::runtime_error("dashDuration must be > 0");
    if (data.airborneFraction <= 0 || data.airborneFraction > 1)
        throw std::runtime_error("airborneFraction must be within (0, 1]");
}

void DashAbility::decide(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    Decided &decided)
{
    decided.dash.emit = false;
    decided.dash.velocity = glm::vec2(0.0f);

    if (observed.contacts.onGround && decided.dash.timeLeft <= 0.0f)
        decided.dash.available = true;

    if (inputIntentions.dashRequested && std::abs(inputIntentions.direction.x) > 0.0f &&
        decided.dash.available && !observed.contacts.touchingLeftWall &&
        !observed.contacts.touchingRightWall)
    {
        decided.dash.direction = inputIntentions.direction.x;
        decided.dash.timeLeft = observed.contacts.onGround
                                    ? data.dashDuration
                                    : data.dashDuration * data.airborneFraction;
        decided.dash.available = false;
        decided.dash.emit = true;
        decided.dash.active = true;
    }

    if (decided.dash.timeLeft > 0.0f && decided.dash.active)
    {
        if (observed.contacts.touchingWall())
        {
            decided.dash.timeLeft = 0.0f;
            decided.dash.active = false;
        }
        else
        {
            decided.dash.timeLeft -= deltaTime;

            if (decided.dash.timeLeft > 0.0f)
                decided.dash.velocity.x = data.dashSpeed * decided.dash.direction;
            else
            {
                decided.dash.timeLeft = 0.0f;
                decided.dash.active = false;
            }
        }
    }
    else if (decided.dash.timeLeft <= 0.0f)
        decided.dash.active = false;
}