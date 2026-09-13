#pragma once

#include <map>
#include <string>
#include "conditions/asked.hpp"

struct WhenData : std::map<std::string, Asked>
{
    bool operator==(const WhenData &) const = default;
};
