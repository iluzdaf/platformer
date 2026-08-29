#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_contact_state.hpp"
#include "actor/abilities/move_ability_state.hpp"
#include "actor/abilities/dash_ability_state.hpp"
#include "actor/abilities/jump_ability_state.hpp"
#include "actor/abilities/wall_slide_ability_state.hpp"
#include "actor/abilities/wall_jump_ability_state.hpp"
#include "actor/abilities/wall_hang_ability_state.hpp"
#include "actor/abilities/wall_climb_ability_state.hpp"
#include "actor/abilities/gravity_ability_state.hpp"

struct ActorMotionState
{
    ActorContactState contacts;

    glm::vec2 previousVelocity = glm::vec2(0.0f), velocity = glm::vec2(0.0f),
              targetVelocity = glm::vec2(0.0f);

    MoveAbilityState move;
    DashAbilityState dash;
    JumpAbilityState jump;
    WallSlideAbilityState wallSlide;
    WallJumpAbilityState wallJump;
    WallHangAbilityState wallHang;
    WallClimbAbilityState wallClimb;
    GravityAbilityState gravity;
};
