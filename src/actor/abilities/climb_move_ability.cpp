#include <stdexcept>
#include "actor/abilities/climb_move_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/climb_move_ability.hpp"
#include "input/input_intentions.hpp"

ClimbMoveAbility::ClimbMoveAbility(const ClimbMoveAbilityData &data) : data(data)
{
    if (data.climbSpeed <= 0)
        throw std::runtime_error("climbSpeed must be greater than 0");
}

void ClimbMoveAbility::applyMovement(
    float,
    const InputIntentions &inputIntentions,
    ActorMotionState &state)
{
    state.climbMove.velocity = glm::vec2(0.0f);

    if (!state.climb.active)
        return;

    if (inputIntentions.direction.y < 0)
        state.climbMove.velocity.y = -data.climbSpeed;
    else if (inputIntentions.direction.y > 0)
        state.climbMove.velocity.y = data.climbSpeed;
}