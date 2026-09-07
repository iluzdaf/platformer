#include <stdexcept>
#include "actor/abilities/move_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/move_ability.hpp"
#include "input/input_intentions.hpp"

MoveAbility::MoveAbility(const MoveAbilityData &data) : data(data)
{
    if (data.moveSpeed <= 0)
        throw std::runtime_error("moveSpeed must be greater than 0");
}

void MoveAbility::decide(
    float,
    const InputIntentions &inputIntentions,
    const Observed &,
    Decided &decided)
{
    decided.move.velocity = glm::vec2(0.0f);

    if (inputIntentions.direction.x > 0)
        decided.move.velocity.x = data.moveSpeed;
    else if (inputIntentions.direction.x < 0)
        decided.move.velocity.x = -data.moveSpeed;
}