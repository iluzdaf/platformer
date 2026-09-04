#pragma once

#include <string>
#include "tile_map/tile_palette_data.hpp"

int tilesInSheet(int textureWidth, int textureHeight, int tileSize);

void checkTileSetFits(
    const TilePaletteData &palette,
    const std::string &paletteName,
    int textureWidth,
    int textureHeight);
