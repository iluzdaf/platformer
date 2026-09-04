#pragma once

#include <string>
#include <vector>
#include <optional>
#include "tile_map/tile_palette_data.hpp"

struct TileMapData
{
    std::optional<int> width;
    std::optional<int> height;
    std::optional<std::vector<std::vector<int>>> indices;
    std::string tilePalette{DefaultTilePalette};
};
