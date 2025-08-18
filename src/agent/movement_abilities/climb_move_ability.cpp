#include <stdexcept>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/climb_move_ability.hpp"
#include "input/input_intentions.hpp"

ClimbMoveAbility::ClimbMoveAbility(const ClimbMoveAbilityData &data)
    : data(data)
{
    if (data.climbSpeed <= 0)
        throw std::runtime_error("climbSpeed must be greater than 0");
}

void ClimbMoveAbility::applyMovement(
    float,
    const InputIntentions &inputIntentions,
    AgentState &state)
{
    state.climbMoveVelocity = glm::vec2(0.0f);

    if (!state.climbing)
        return;

    if (inputIntentions.direction.y < 0)
        state.climbMoveVelocity.y = -data.climbSpeed;
    else if (inputIntentions.direction.y > 0)
        state.climbMoveVelocity.y = data.climbSpeed;
}