#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/wall_hang_ability.hpp"
#include "input/input_intentions.hpp"

WallHangAbility::WallHangAbility(const WallHangAbilityData &)
{
}

void WallHangAbility::applyMovement(
    float,
    const InputIntentions &inputIntentions,
    ActorMotionState &state)
{
    state.wallHang.active = false;

    if (!inputIntentions.climbRequested)
        return;

    if (!(state.contacts.touchingLeftWall || state.contacts.touchingRightWall))
        return;

    state.wallHang.active = true;
}