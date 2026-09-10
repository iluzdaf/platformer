#pragma once

#include <map>
#include <optional>
#include <string>
#include "tile_map/tile_data.hpp"
#include "assets/sheet_data.hpp"

struct TilePaletteData
{
    SheetData tileSet;
    std::optional<int> tileSize;
    std::map<int, TileData> tiles;
};

using TilePalettes = std::map<std::string, TilePaletteData>;

inline int tileSizeOf(const TilePaletteData &palette)
{
    return palette.tileSize.value_or(palette.tileSet.cellSize.x);
}
