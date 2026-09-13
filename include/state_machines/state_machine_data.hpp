#pragma once

#include <string>
#include <vector>
#include "conditions/when_data.hpp"

struct TransitionData
{
    std::string from;
    std::string to;
    WhenData when;
    float after = 0.0f;

    bool operator==(const TransitionData &) const = default;
};

template <class Does> struct StateData
{
    std::string name;
    Does does;
    float cooldown = 0.0f;

    bool operator==(const StateData &) const = default;
};

template <class Does> struct StateMachineData
{
    std::vector<StateData<Does>> states;
    std::vector<TransitionData> transitions;

    bool operator==(const StateMachineData &) const = default;
};
