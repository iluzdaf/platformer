#pragma once

#include <optional>
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/abilities/climb_ability_data.hpp"
#include "actor/abilities/climb_move_ability_data.hpp"
#include "actor/abilities/gravity_ability_data.hpp"

struct ActorMotionData
{
    std::optional<MoveAbilityData> moveAbilityData;
    std::optional<JumpAbilityData> jumpAbilityData;
    std::optional<DashAbilityData> dashAbilityData;
    std::optional<WallSlideAbilityData> wallSlideAbilityData;
    std::optional<WallJumpAbilityData> wallJumpAbilityData;
    std::optional<ClimbAbilityData> climbAbilityData;
    std::optional<ClimbMoveAbilityData> climbMoveAbilityData;
    std::optional<GravityAbilityData> gravityAbilityData;
};
