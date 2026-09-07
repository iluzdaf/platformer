#include "actor/actor_motion_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/abilities.hpp"
#include "actor/abilities/move_ability.hpp"
#include "actor/abilities/jump_ability.hpp"
#include "actor/abilities/dash_ability.hpp"
#include "actor/abilities/wall_slide_ability.hpp"
#include "actor/abilities/wall_jump_ability.hpp"
#include "actor/abilities/wall_hang_ability.hpp"
#include "actor/abilities/wall_climb_ability.hpp"
#include "actor/abilities/mantle_ability.hpp"
#include "actor/abilities/gravity_ability.hpp"
#include "actor/abilities/knockback_ability.hpp"
#include <memory>

Abilities::Abilities(const ActorMotionData &data)
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
    if (data.wallHangAbilityData)
        abilities.push_back(std::make_unique<WallHangAbility>(data.wallHangAbilityData.value()));
    if (data.wallClimbAbilityData)
        abilities.push_back(std::make_unique<WallClimbAbility>(data.wallClimbAbilityData.value()));
    if (data.mantleAbilityData)
        abilities.push_back(std::make_unique<MantleAbility>(data.mantleAbilityData.value()));
    if (data.gravityAbilityData)
        abilities.push_back(std::make_unique<GravityAbility>(data.gravityAbilityData.value()));
    if (data.knockbackAbilityData)
        abilities.push_back(std::make_unique<KnockbackAbility>(data.knockbackAbilityData.value()));
}

void Abilities::decide(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    Decided &decided)
{
    for (auto &ability : abilities)
        ability->decide(deltaTime, inputIntentions, observed, decided);

    glm::vec2 finalVelocity = decided.gravity.velocity;
    if (decided.knockback.active)
        finalVelocity = decided.knockback.velocity;
    else if (decided.dash.active)
        finalVelocity = decided.dash.velocity;
    else if (decided.mantle.active)
        finalVelocity = decided.mantle.velocity;
    else
    {
        finalVelocity.x = decided.move.velocity.x;

        if (decided.jump.active)
            finalVelocity.y = decided.jump.velocity.y;
        else if (decided.wallJump.active)
            finalVelocity = decided.wallJump.velocity;
        else if (decided.wallHang.active)
            finalVelocity.y = decided.wallClimb.velocity.y;
        else if (decided.wallSlide.active)
            finalVelocity.y = decided.wallSlide.velocity.y;
    }

    decided.targetVelocity = finalVelocity;
}