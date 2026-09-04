#pragma once

#include <map>
#include <string>
#include "tile_map/tile_data.hpp"
#include "assets/sheet_data.hpp"

struct TilePaletteData
{
    SheetData tileSet;
    std::map<int, TileData> tiles;
};

using TilePalettes = std::map<std::string, TilePaletteData>;
