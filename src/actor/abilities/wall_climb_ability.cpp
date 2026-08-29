#include <stdexcept>
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/wall_climb_ability.hpp"
#include "input/input_intentions.hpp"

WallClimbAbility::WallClimbAbility(const WallClimbAbilityData &data) : data(data)
{
    if (data.climbSpeed <= 0)
        throw std::runtime_error("climbSpeed must be greater than 0");
}

void WallClimbAbility::applyMovement(
    float,
    const InputIntentions &inputIntentions,
    ActorMotionState &state)
{
    state.wallClimb.velocity = glm::vec2(0.0f);

    if (!state.wallHang.active)
        return;

    if (inputIntentions.direction.y < 0)
        state.wallClimb.velocity.y = -data.climbSpeed;
    else if (inputIntentions.direction.y > 0)
        state.wallClimb.velocity.y = data.climbSpeed;
}