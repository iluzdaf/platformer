#pragma once

struct Brush
{
    enum class Kind
    {
        Tile,
        PlayerStart
    };

    Kind kind = Kind::Tile;
    int tileIndex = 0;

    bool operator==(const Brush &) const = default;
};
