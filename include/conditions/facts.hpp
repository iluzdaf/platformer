#pragma once

#include <map>
#include <optional>
#include <string>
#include "conditions/asked.hpp"

struct Facts : std::map<std::string, Asked>
{
    bool operator==(const Facts &) const = default;
};

std::optional<std::string> whyNotDeclared(
    const Facts &declared,
    const std::string &name,
    const Asked &value);
