#pragma once

#include <span>
#include <string_view>
#include "conditions/fact_rows.hpp"

struct AbilityStates;
struct Observed;

struct AnimatorFacts
{
    const AbilityStates &abilityStates;
    const Observed &observed;
    bool finished;
    std::string_view inState;
};

std::span<const FactRow<AnimatorFacts>> animatorRows();
