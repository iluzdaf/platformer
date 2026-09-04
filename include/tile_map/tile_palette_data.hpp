#pragma once

#include <map>
#include <string>
#include <string_view>
#include "tile_map/tile_data.hpp"
#include "assets/sheet_data.hpp"

inline constexpr std::string_view DefaultTilePalette = "default";

struct TilePaletteData
{
    SheetData tileSet;
    std::map<int, TileData> tiles;
};

using TilePalettes = std::map<std::string, TilePaletteData>;
