#pragma once

#include <span>
#include <string>
#include <vector>
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "conditions/facts.hpp"
#include "ui/in_scope.hpp"

struct FactOffered
{
    std::string name;
    AskedKind kind;
};

using FactsOffered = std::vector<FactOffered>;

template <class Context>
FactsOffered factsOffered(
    std::span<const FactRow<Context>> rows,
    const FactsData *declared = nullptr)
{
    FactsOffered offered;
    for (const FactRow<Context> &row : rows)
        offered.push_back({std::string(row.name), row.kind});

    if (declared)
        for (const auto &[name, value] : *declared)
            offered.push_back({name, kindOf(value)});

    return offered;
}

inline const FactsOffered *factsOfferedInScope()
{
    return inScope<FactsOffered>();
}
