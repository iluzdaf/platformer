#pragma once

#include <optional>

struct TilePickupData
{
    int replaceIndex = 0;
    std::optional<int> scoreDelta;

    bool operator==(const TilePickupData &) const = default;
};
