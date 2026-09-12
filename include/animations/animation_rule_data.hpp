#pragma once

#include <string>
#include "conditions/asked.hpp"

struct AnimationRuleData
{
    std::string show;
    AnimationWhenData when;

    bool operator==(const AnimationRuleData &) const = default;
};
