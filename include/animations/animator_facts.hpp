#pragma once

#include <span>
#include <string_view>
#include "conditions/fact_rows.hpp"

struct Decided;
struct Observed;

struct AnimatorFacts
{
    const Decided &decided;
    const Observed &observed;
    bool finished;
    std::string_view inState;
};

std::span<const FactRow<AnimatorFacts>> animatorRows();
