#pragma once

#include <string>
#include "tile_map/tile_palette.hpp"

int tilesInSheet(int textureWidth, int textureHeight, int tileSize);

void checkTileSetFits(
    const TilePalette &palette,
    const std::string &paletteName,
    int textureWidth,
    int textureHeight);
