#include <stdexcept>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/dash_ability.hpp"
#include "input/input_intentions.hpp"

DashAbility::DashAbility(const DashAbilityData &data)
    : data(data)
{
    if (data.dashSpeed <= 0)
        throw std::runtime_error("dashSpeed must be > 0");
    if (data.dashDuration <= 0)
        throw std::runtime_error("dashDuration must be > 0");
}

void DashAbility::applyMovement(
    float deltaTime,
    const InputIntentions &inputIntentions,
    AgentState &state)
{
    state.emitDash = false;
    state.dashVelocity = glm::vec2(0.0f);

    if (state.onGround && state.dashTimeLeft <= 0.0f)
        state.canDash = true;

    if (inputIntentions.dashRequested &&
        std::abs(inputIntentions.direction.x) > 0.0f &&
        state.canDash &&
        !state.touchingLeftWall &&
        !state.touchingRightWall)
    {
        state.dashDirection = inputIntentions.direction.x;
        state.dashTimeLeft = data.dashDuration;
        state.canDash = false;
        state.emitDash = true;
        state.dashing = true;
    }

    if (state.dashTimeLeft > 0.0f && state.dashing)
    {
        if (state.touchingLeftWall || state.touchingRightWall)
        {
            state.dashTimeLeft = 0.0f;
            state.dashing = false;
        }
        else
        {
            state.dashTimeLeft -= deltaTime;

            if (state.dashTimeLeft > 0.0f)
                state.dashVelocity.x = data.dashSpeed * state.dashDirection;
            else
            {
                state.dashTimeLeft = 0.0f;
                state.dashing = false;
            }
        }
    }
    else if (state.dashTimeLeft <= 0.0f)
        state.dashing = false;
}