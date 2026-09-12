#include <stdexcept>
#include "actor/abilities/ability_states.hpp"
#include "actor/abilities/bite_ability.hpp"
#include "actor/abilities/bite_ability_data.hpp"

BiteAbility::BiteAbility(const BiteAbilityData &data) : data(data)
{
    if (data.damage <= 0)
        throw std::runtime_error("A bite needs damage above 0");
}

void BiteAbility::decide(float, const InputIntentions &, const Observed &, AbilityStates &states)
{
    states.bite.active = true;
    states.bite.damage = data.damage;
}
