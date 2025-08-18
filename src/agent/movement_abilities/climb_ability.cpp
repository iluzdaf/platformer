#include <stdexcept>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/climb_ability.hpp"
#include "input/input_intentions.hpp"

ClimbAbility::ClimbAbility(const ClimbAbilityData &)
{
}

void ClimbAbility::applyMovement(
    float,
    const InputIntentions &inputIntentions,
    AgentState &state)
{
    state.climbing = false;

    if (!inputIntentions.climbRequested)
        return;

    if (!(state.touchingLeftWall || state.touchingRightWall))
        return;

    state.climbing = true;
}