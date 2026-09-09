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

struct ActorMotionData
{
    std::optional<MoveAbilityData> moveAbilityData;
    std::optional<JumpAbilityData> jumpAbilityData;
    std::optional<DashAbilityData> dashAbilityData;
    std::optional<WallSlideAbilityData> wallSlideAbilityData;
    std::optional<WallJumpAbilityData> wallJumpAbilityData;
    std::optional<WallHangAbilityData> wallHangAbilityData;
    std::optional<WallClimbAbilityData> wallClimbAbilityData;
    std::optional<MantleAbilityData> mantleAbilityData;
    std::optional<GravityAbilityData> gravityAbilityData;
    std::optional<KnockbackAbilityData> knockbackAbilityData;
    std::optional<SwingAbilityData> swingAbilityData;
    std::optional<PounceAbilityData> pounceAbilityData;

    bool operator==(const ActorMotionData &) const = default;
};
