#pragma once

#include <optional>
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/abilities/knockback_ability_data.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/charge_ability_data.hpp"

struct AbilitiesData
{
    std::optional<MoveAbilityData> move;
    std::optional<JumpAbilityData> jump;
    std::optional<DashAbilityData> dash;
    std::optional<WallSlideAbilityData> wallSlide;
    std::optional<WallJumpAbilityData> wallJump;
    std::optional<WallHangAbilityData> wallHang;
    std::optional<WallClimbAbilityData> wallClimb;
    std::optional<MantleAbilityData> mantle;
    std::optional<GravityAbilityData> gravity;
    std::optional<KnockbackAbilityData> knockback;
    std::optional<SwingAbilityData> swing;
    std::optional<PounceAbilityData> pounce;
    std::optional<ChargeAbilityData> charge;

    bool operator==(const AbilitiesData &) const = default;
};
