#include <cmath>
#include "animations/animation_parameters.hpp"
#include "actor/abilities/dash_ability_state.hpp"
#include "actor/abilities/knockback_ability_state.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/abilities/wall_climb_ability_state.hpp"
#include "actor/abilities/wall_hang_ability_state.hpp"
#include "actor/abilities/wall_slide_ability_state.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"

namespace
{
    constexpr float StandingStill = 0.1f;
}

AnimationParameters parametersFrom(const Decided &decided, const Observed &observed, bool finished)
{
    AnimationParameters parameters;
    parameters.alive = observed.alive;
    parameters.knockback = decided.knockback.active;
    parameters.swinging = decided.swing.swinging();
    parameters.dashing = decided.dash.active;
    parameters.onGround = observed.contacts.onGround;
    parameters.climbing = decided.wallHang.active && decided.wallClimb.velocity.y != 0.0f;
    parameters.onWall = decided.wallSlide.active || decided.wallHang.active;
    parameters.rising = observed.velocity.y < 0.0f;
    parameters.falling = observed.velocity.y > 0.0f;
    parameters.moving = std::abs(observed.velocity.x) > StandingStill;
    parameters.finished = finished;
    return parameters;
}
