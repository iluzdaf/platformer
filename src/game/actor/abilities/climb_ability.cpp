#include <stdexcept>
#include "game/actor/actor_motion_state.hpp"
#include "game/actor/abilities/climb_ability.hpp"
#include "input/input_intentions.hpp"

ClimbAbility::ClimbAbility(const ClimbAbilityData &)
{
}

void ClimbAbility::applyMovement(
    float,
    const InputIntentions &inputIntentions,
    ActorMotionState &state)
{
    state.climbing = false;

    if (!inputIntentions.climbRequested)
        return;

    if (!(state.touchingLeftWall || state.touchingRightWall))
        return;

    state.climbing = true;
}