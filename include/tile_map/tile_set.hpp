#pragma once

#include <string>

struct TileSet
{
    std::string texture;
    int tileSize = 16;

    bool operator==(const TileSet &) const = default;
};
