#pragma once

#include <string>
#include <vector>
#include "conditions/asked.hpp"

struct AnimationTransitionData
{
    std::string from;
    std::string to;
    AnimationWhenData when;

    bool operator==(const AnimationTransitionData &) const = default;
};

struct AnimationLadderData
{
    std::vector<AnimationTransitionData> transitions;

    bool operator==(const AnimationLadderData &) const = default;
};
