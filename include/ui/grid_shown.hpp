#pragma once

#include <optional>

struct GridShown
{
    bool showing = false;
    std::optional<bool> beforeArming;

    bool operator==(const GridShown &) const = default;
};

inline GridShown whileArmed(GridShown grid, bool armed)
{
    if (armed && !grid.beforeArming)
        return GridShown{true, grid.showing};

    if (!armed && grid.beforeArming)
        return GridShown{*grid.beforeArming, std::nullopt};

    return grid;
}
