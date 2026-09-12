#include "actor/abilities/abilities_data.hpp"
#include "actor/abilities/ability_states.hpp"
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
#include "actor/abilities/bite_ability.hpp"
#include <memory>

Abilities::Abilities(const AbilitiesData &data)
{
    if (data.move)
        abilities.push_back(std::make_unique<MoveAbility>(data.move.value()));
    if (data.jump)
        abilities.push_back(std::make_unique<JumpAbility>(data.jump.value()));
    if (data.dash)
        abilities.push_back(std::make_unique<DashAbility>(data.dash.value()));
    if (data.wallSlide)
        abilities.push_back(std::make_unique<WallSlideAbility>(data.wallSlide.value()));
    if (data.wallJump)
        abilities.push_back(std::make_unique<WallJumpAbility>(data.wallJump.value()));
    if (data.wallHang)
        abilities.push_back(std::make_unique<WallHangAbility>(data.wallHang.value()));
    if (data.wallClimb)
        abilities.push_back(std::make_unique<WallClimbAbility>(data.wallClimb.value()));
    if (data.mantle)
        abilities.push_back(std::make_unique<MantleAbility>(data.mantle.value()));
    if (data.gravity)
        abilities.push_back(std::make_unique<GravityAbility>(data.gravity.value()));
    if (data.pounce)
        abilities.push_back(std::make_unique<PounceAbility>(data.pounce.value()));
    if (data.charge)
        abilities.push_back(std::make_unique<ChargeAbility>(data.charge.value()));
    if (data.bite)
        abilities.push_back(std::make_unique<BiteAbility>(data.bite.value()));
    if (data.knockback)
        abilities.push_back(std::make_unique<KnockbackAbility>(data.knockback.value()));
    if (data.swing)
        abilities.push_back(std::make_unique<SwingAbility>(data.swing.value()));
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