#include "actor/actor_motion_data.hpp"
#include "actor/ability_states.hpp"
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
#include "actor/abilities/swing_ability.hpp"
#include "actor/abilities/pounce_ability.hpp"
#include "actor/abilities/charge_ability.hpp"
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
    if (data.pounceAbilityData)
        abilities.push_back(std::make_unique<PounceAbility>(data.pounceAbilityData.value()));
    if (data.chargeAbilityData)
        abilities.push_back(std::make_unique<ChargeAbility>(data.chargeAbilityData.value()));
    if (data.knockbackAbilityData)
        abilities.push_back(std::make_unique<KnockbackAbility>(data.knockbackAbilityData.value()));
    if (data.swingAbilityData)
        abilities.push_back(std::make_unique<SwingAbility>(data.swingAbilityData.value()));
}

glm::vec2 Abilities::decide(
    float deltaTime,
    const InputIntentions &inputIntentions,
    const Observed &observed,
    AbilityStates &states)
{
    for (auto &ability : abilities)
        ability->decide(deltaTime, inputIntentions, observed, states);

    glm::vec2 velocity = states.gravity.velocity;
    if (states.knockback.active)
        velocity = states.knockback.velocity;
    else if (states.dash.active)
        velocity = states.dash.velocity;
    else if (states.mantle.active)
        velocity = states.mantle.velocity;
    else if (states.pounce.active)
        velocity = states.pounce.velocity;
    else if (states.charge.active)
        velocity = states.charge.velocity;
    else
    {
        velocity.x = states.move.velocity.x;

        if (states.jump.active)
            velocity.y = states.jump.velocity.y;
        else if (states.wallJump.active)
            velocity = states.wallJump.velocity;
        else if (states.wallHang.active)
            velocity.y = states.wallClimb.velocity.y;
        else if (states.wallSlide.active)
            velocity.y = states.wallSlide.velocity.y;
    }

    return velocity;
}