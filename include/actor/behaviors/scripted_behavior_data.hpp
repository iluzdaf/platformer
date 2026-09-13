#pragma once

#include <string>

struct ScriptedBehaviorData
{
    std::string call;

    bool operator==(const ScriptedBehaviorData &) const = default;
};
