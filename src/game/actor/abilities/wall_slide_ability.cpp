#include <stdexcept>
#include "game/actor/actor_motion_state.hpp"
#include "game/actor/abilities/wall_slide_ability.hpp"
#include "input/input_intentions.hpp"

WallSlideAbility::WallSlideAbility(const WallSlideAbilityData &data)
    : data(data)
{
    if (data.slideSpeed <= 0)
        throw std::runtime_error("slideSpeed must be positive");
}

void WallSlideAbility::applyMovement(
    float,
    const InputIntentions &,
    ActorMotionState &state)
{
    state.emitWallSliding = false;
    state.wallSlideVelocity = glm::vec2(0.0f);
    state.wallSliding = false;

    if (state.onGround ||
        !(state.touchingLeftWall || state.touchingRightWall) ||
        state.velocity.y <= 0.0f)
        return;

    state.wallSlideVelocity.y = data.slideSpeed;
    state.wallSliding = true;
    state.emitWallSliding = true;
}