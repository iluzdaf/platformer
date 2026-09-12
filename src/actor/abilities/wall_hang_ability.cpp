#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/wall_hang_ability.hpp"
#include "input/input_intentions.hpp"

WallHangAbility::WallHangAbility(const WallHangAbilityData &)
{
}

void WallHangAbility::decide(
    float,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    AbilityStates &states)
{
    states.wallHang.active = false;

    if (!inputIntentions.climbRequested)
        return;

    if (!observed.contacts.grippableWall())
        return;

    states.wallHang.active = true;
}