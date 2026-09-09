#pragma once

#include <string>
#include <vector>
#include "conditions/asked.hpp"

struct AnimationTransitionData
{
    std::string from;
    std::string to;
    AnimationWhen when;

    bool operator==(const AnimationTransitionData &) const = default;
};

struct AnimatorData
{
    std::vector<AnimationTransitionData> transitions;

    bool operator==(const AnimatorData &) const = default;
};
