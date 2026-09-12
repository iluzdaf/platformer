#pragma once

#include "actor/abilities/move_ability_state.hpp"
#include "actor/abilities/dash_ability_state.hpp"
#include "actor/abilities/jump_ability_state.hpp"
#include "actor/abilities/wall_slide_ability_state.hpp"
#include "actor/abilities/wall_jump_ability_state.hpp"
#include "actor/abilities/wall_hang_ability_state.hpp"
#include "actor/abilities/wall_climb_ability_state.hpp"
#include "actor/abilities/mantle_ability_state.hpp"
#include "actor/abilities/gravity_ability_state.hpp"
#include "actor/abilities/knockback_ability_state.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/abilities/bite_ability_state.hpp"

struct AbilityStates
{
    MoveAbilityState move;
    DashAbilityState dash;
    JumpAbilityState jump;
    WallSlideAbilityState wallSlide;
    WallJumpAbilityState wallJump;
    WallHangAbilityState wallHang;
    WallClimbAbilityState wallClimb;
    MantleAbilityState mantle;
    GravityAbilityState gravity;
    KnockbackAbilityState knockback;
    SwingAbilityState swing;
    PounceAbilityState pounce;
    ChargeAbilityState charge;
    BiteAbilityState bite;
};
