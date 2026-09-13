#pragma once

#include <string>
#include "conditions/when_data.hpp"

struct AnimationRuleData
{
    std::string show;
    WhenData when;

    bool operator==(const AnimationRuleData &) const = default;
};
