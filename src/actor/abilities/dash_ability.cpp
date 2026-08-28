#include <stdexcept>
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/dash_ability.hpp"
#include "input/input_intentions.hpp"

DashAbility::DashAbility(const DashAbilityData &data)
    : data(data)
{
    if (data.dashSpeed <= 0)
        throw std::runtime_error("dashSpeed must be > 0");
    if (data.dashDuration <= 0)
        throw std::runtime_error("dashDuration must be > 0");
    if (data.airborneFraction <= 0 || data.airborneFraction > 1)
        throw std::runtime_error("airborneFraction must be within (0, 1]");
}

void DashAbility::applyMovement(
    float deltaTime,
    const InputIntentions &inputIntentions,
    ActorMotionState &state)
{
    state.dash.emit = false;
    state.dash.velocity = glm::vec2(0.0f);

    if (state.contacts.onGround && state.dash.timeLeft <= 0.0f)
        state.dash.available = true;

    if (inputIntentions.dashRequested &&
        std::abs(inputIntentions.direction.x) > 0.0f &&
        state.dash.available &&
        !state.contacts.touchingLeftWall &&
        !state.contacts.touchingRightWall)
    {
        state.dash.direction = inputIntentions.direction.x;
        state.dash.timeLeft = state.contacts.onGround
                                  ? data.dashDuration
                                  : data.dashDuration * data.airborneFraction;
        state.dash.available = false;
        state.dash.emit = true;
        state.dash.active = true;
    }

    if (state.dash.timeLeft > 0.0f && state.dash.active)
    {
        if (state.contacts.touchingLeftWall || state.contacts.touchingRightWall)
        {
            state.dash.timeLeft = 0.0f;
            state.dash.active = false;
        }
        else
        {
            state.dash.timeLeft -= deltaTime;

            if (state.dash.timeLeft > 0.0f)
                state.dash.velocity.x = data.dashSpeed * state.dash.direction;
            else
            {
                state.dash.timeLeft = 0.0f;
                state.dash.active = false;
            }
        }
    }
    else if (state.dash.timeLeft <= 0.0f)
        state.dash.active = false;
}