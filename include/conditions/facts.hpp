#pragma once

#include <map>
#include <optional>
#include <string>
#include "conditions/asked.hpp"

struct FactsData : std::map<std::string, Asked>
{
    bool operator==(const FactsData &) const = default;
};

std::optional<std::string> whyNotDeclared(
    const FactsData &declared,
    const std::string &name,
    const Asked &value);
