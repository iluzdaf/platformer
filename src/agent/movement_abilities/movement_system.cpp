#include "agent/agent_data.hpp"
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/movement_system.hpp"
#include "agent/movement_abilities/move_ability.hpp"
#include "agent/movement_abilities/jump_ability.hpp"
#include "agent/movement_abilities/dash_ability.hpp"
#include "agent/movement_abilities/wall_slide_ability.hpp"
#include "agent/movement_abilities/wall_jump_ability.hpp"
#include "agent/movement_abilities/climb_ability.hpp"
#include "agent/movement_abilities/climb_move_ability.hpp"
#include "agent/movement_abilities/gravity_ability.hpp"

MovementSystem::MovementSystem(const AgentData &data)
{
    if (data.moveAbilityData)
        abilities.push_back(std::make_unique<MoveAbility>(data.moveAbilityData.value()));
    if (data.jumpAbilityData)
        abilities.push_back(std::make_unique<JumpAbility>(data.jumpAbilityData.value()));
    if (data.dashAbilityData)
        abilities.push_back(std::make_unique<DashAbility>(data.dashAbilityData.value()));
    if (data.wallSlideAbilityData)
        abilities.push_back(std::make_unique<WallSlideAbility>(data.wallSlideAbilityData.value()));
    if (data.wallJumpAbilityData)
        abilities.push_back(std::make_unique<WallJumpAbility>(data.wallJumpAbilityData.value()));
    if (data.climbAbilityData)
        abilities.push_back(std::make_unique<ClimbAbility>(data.climbAbilityData.value()));
    if (data.climbMoveAbilityData)
        abilities.push_back(std::make_unique<ClimbMoveAbility>(data.climbMoveAbilityData.value()));
    if (data.gravityAbilityData)
        abilities.push_back(std::make_unique<GravityAbility>(data.gravityAbilityData.value()));
}

void MovementSystem::applyMovement(
    float deltaTime,
    const InputIntentions &inputIntentions,
    AgentState &state)
{
    for (auto &ability : abilities)
        ability->applyMovement(deltaTime, inputIntentions, state);

    glm::vec2 finalVelocity = state.gravityVelocity;

    if (state.dashing)
        finalVelocity = state.dashVelocity;
    else
    {
        finalVelocity.x = state.moveVelocity.x;

        if (state.jumping)
            finalVelocity.y = state.jumpVelocity.y;
        else if (state.wallJumping)
            finalVelocity = state.wallJumpVelocity;
        else if (state.climbing)
            finalVelocity.y = state.climbMoveVelocity.y;
        else if (state.wallSliding)
            finalVelocity.y = state.wallSlideVelocity.y;
    }

    state.targetVelocity = finalVelocity;
}