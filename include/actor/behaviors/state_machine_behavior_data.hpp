#pragma once

#include <optional>
#include <string>
#include <vector>
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"

struct BehaviorStateData
{
    std::string name;
    std::optional<PatrolBehaviorData> patrolBehaviorData;
    std::optional<FleeBehaviorData> fleeBehaviorData;

    bool operator==(const BehaviorStateData &) const = default;
};

struct BehaviorTransitionData
{
    std::string from;
    std::string to;
    std::optional<float> threatWithin;
    std::optional<float> threatBeyond;
    float after = 0.0f;

    bool operator==(const BehaviorTransitionData &) const = default;
};

struct StateMachineBehaviorData
{
    std::vector<BehaviorStateData> states;
    std::vector<BehaviorTransitionData> transitions;

    bool operator==(const StateMachineBehaviorData &) const = default;
};
