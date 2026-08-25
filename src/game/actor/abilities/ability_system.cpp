#include "game/actor/actor_motion_data.hpp"
#include "game/actor/actor_motion_state.hpp"
#include "game/actor/abilities/ability_system.hpp"
#include "game/actor/abilities/move_ability.hpp"
#include "game/actor/abilities/jump_ability.hpp"
#include "game/actor/abilities/dash_ability.hpp"
#include "game/actor/abilities/wall_slide_ability.hpp"
#include "game/actor/abilities/wall_jump_ability.hpp"
#include "game/actor/abilities/climb_ability.hpp"
#include "game/actor/abilities/climb_move_ability.hpp"
#include "game/actor/abilities/gravity_ability.hpp"

AbilitySystem::AbilitySystem(const ActorMotionData &data)
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

void AbilitySystem::applyMovement(
    float deltaTime,
    const InputIntentions &inputIntentions,
    ActorMotionState &state)
{
    for (auto &ability : abilities)
        ability->applyMovement(deltaTime, inputIntentions, state);

    glm::vec2 finalVelocity = state.gravity.velocity;

    if (state.dash.active)
        finalVelocity = state.dash.velocity;
    else
    {
        finalVelocity.x = state.move.velocity.x;

        if (state.jump.active)
            finalVelocity.y = state.jump.velocity.y;
        else if (state.wallJump.active)
            finalVelocity = state.wallJump.velocity;
        else if (state.climb.active)
            finalVelocity.y = state.climbMove.velocity.y;
        else if (state.wallSlide.active)
            finalVelocity.y = state.wallSlide.velocity.y;
    }

    state.targetVelocity = finalVelocity;
}