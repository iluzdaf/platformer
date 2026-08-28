#include "actor/abilities/climb_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/climb_ability.hpp"
#include "input/input_intentions.hpp"

ClimbAbility::ClimbAbility(const ClimbAbilityData &)
{
}

void ClimbAbility::applyMovement(
    float,
    const InputIntentions &inputIntentions,
    ActorMotionState &state)
{
    state.climb.active = false;

    if (!inputIntentions.climbRequested)
        return;

    if (!(state.contacts.touchingLeftWall || state.contacts.touchingRightWall))
        return;

    state.climb.active = true;
}