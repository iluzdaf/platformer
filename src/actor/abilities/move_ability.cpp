#include <stdexcept>
#include "actor/abilities/move_ability_data.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/move_ability.hpp"
#include "input/input_intentions.hpp"

MoveAbility::MoveAbility(const MoveAbilityData &data) : data(data)
{
    if (data.moveSpeed <= 0)
        throw std::runtime_error("A move needs a speed above 0");
}

void MoveAbility::decide(
    float,
    const InputIntentions &inputIntentions,
    const Observed &,
    AbilityStates &states)
{
    states.move.velocity = glm::vec2(0.0f);

    if (inputIntentions.direction.x > 0)
        states.move.velocity.x = data.moveSpeed;
    else if (inputIntentions.direction.x < 0)
        states.move.velocity.x = -data.moveSpeed;
}