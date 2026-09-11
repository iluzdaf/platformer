#pragma once

#include "ui/in_scope.hpp"

struct FactsData;

inline const FactsData *factsInScope()
{
    return inScope<FactsData>();
}
