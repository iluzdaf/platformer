#pragma once

#include <optional>

struct SensesData
{
    std::optional<float> close;
    std::optional<float> reach;

    bool operator==(const SensesData &) const = default;
};
