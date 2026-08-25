#include <stdexcept>
#include "game/actor/actor_motion_state.hpp"
#include "game/actor/abilities/climb_move_ability.hpp"
#include "input/input_intentions.hpp"

ClimbMoveAbility::ClimbMoveAbility(const ClimbMoveAbilityData &data)
    : data(data)
{
    if (data.climbSpeed <= 0)
        throw std::runtime_error("climbSpeed must be greater than 0");
}

void ClimbMoveAbility::applyMovement(
    float,
    const InputIntentions &inputIntentions,
    ActorMotionState &state)
{
    state.climbMoveVelocity = glm::vec2(0.0f);

    if (!state.climbing)
        return;

    if (inputIntentions.direction.y < 0)
        state.climbMoveVelocity.y = -data.climbSpeed;
    else if (inputIntentions.direction.y > 0)
        state.climbMoveVelocity.y = data.climbSpeed;
}