#pragma once

#include <string>
#include "tile_map/tile_palette.hpp"

class TextureCache;

void checkTileSetFits(
    const TilePalette &palette,
    const std::string &paletteName,
    int textureWidth,
    int textureHeight);

void warmTileSets(TextureCache &textures, const TilePalettes &tilePalettes);
