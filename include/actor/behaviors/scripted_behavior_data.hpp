#pragma once

#include <string>

struct ScriptedBehaviorData
{
    std::string call;
    float arrivalThreshold = 2.0f;

    bool operator==(const ScriptedBehaviorData &) const = default;
};
