#include <stdexcept>
#include "game/actor/actor_motion_state.hpp"
#include "game/actor/abilities/move_ability.hpp"
#include "input/input_intentions.hpp"

MoveAbility::MoveAbility(const MoveAbilityData &data)
    : data(data)
{
    if (data.moveSpeed <= 0)
        throw std::runtime_error("moveSpeed must be greater than 0");
}

void MoveAbility::applyMovement(
    float,
    const InputIntentions &inputIntentions,
    ActorMotionState &state)
{
    state.moveVelocity = glm::vec2(0.0f);

    if (inputIntentions.direction.x > 0)
        state.moveVelocity.x = data.moveSpeed;
    else if (inputIntentions.direction.x < 0)
        state.moveVelocity.x = -data.moveSpeed;
}