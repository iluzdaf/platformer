#pragma once

#include <string>
#include "assets/asset_paths.hpp"

struct TileSet
{
    std::string texture{assets::TileSetTexture};
    int tileSize = 16;

    bool operator==(const TileSet &) const = default;
};
