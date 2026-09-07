#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/wall_hang_ability.hpp"
#include "input/input_intentions.hpp"

WallHangAbility::WallHangAbility(const WallHangAbilityData &)
{
}

void WallHangAbility::applyMovement(
    float,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    Decided &state)
{
    state.wallHang.active = false;

    if (!inputIntentions.climbRequested)
        return;

    if (!observed.contacts.grippableWall())
        return;

    state.wallHang.active = true;
}