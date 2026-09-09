#pragma once

#include <string>

struct AttackBehaviorData
{
    std::string with;

    bool operator==(const AttackBehaviorData &) const = default;
};
