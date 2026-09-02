#pragma once

struct TileIndex
{
    constexpr TileIndex() = default;
    constexpr TileIndex(int value) : value(value)
    {
    }

    int value = 0;

    bool operator==(const TileIndex &) const = default;
};
