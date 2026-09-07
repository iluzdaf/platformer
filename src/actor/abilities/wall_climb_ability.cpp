#include <stdexcept>
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
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
    const Observed &,
    Decided &decided)
{
    decided.wallClimb.velocity = glm::vec2(0.0f);

    if (!decided.wallHang.active)
        return;

    if (inputIntentions.direction.y < 0)
        decided.wallClimb.velocity.y = -data.climbSpeed;
    else if (inputIntentions.direction.y > 0)
        decided.wallClimb.velocity.y = data.climbSpeed;
}