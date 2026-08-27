#pragma once

#include <map>
#include <string>
#include "tile_map/tile_data.hpp"

using TilePalette = std::map<int, TileData>;
using TilePalettes = std::map<std::string, TilePalette>;
