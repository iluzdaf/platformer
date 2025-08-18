#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include "serialization/glm_vec2_meta.hpp"
#include "agent/movement_abilities/move_ability_data.hpp"
#include "agent/movement_abilities/jump_ability_data.hpp"
#include "agent/movement_abilities/dash_ability_data.hpp"
#include "agent/movement_abilities/wall_slide_ability_data.hpp"
#include "agent/movement_abilities/wall_jump_ability_data.hpp"
#include "agent/movement_abilities/climb_ability_data.hpp"
#include "agent/movement_abilities/climb_move_ability_data.hpp"
#include "agent/movement_abilities/gravity_ability_data.hpp"
#include "physics/physics_body_data.hpp"

struct AgentData
{
    glm::vec2 size = glm::vec2(16, 16);
    std::optional<MoveAbilityData> moveAbilityData;
    std::optional<JumpAbilityData> jumpAbilityData;
    std::optional<DashAbilityData> dashAbilityData;
    std::optional<WallSlideAbilityData> wallSlideAbilityData;
    std::optional<WallJumpAbilityData> wallJumpAbilityData;
    std::optional<ClimbAbilityData> climbAbilityData;
    std::optional<ClimbMoveAbilityData> climbMoveAbilityData;
    std::optional<GravityAbilityData> gravityAbilityData;
    PhysicsBodyData physicsBodyData;
};