#pragma once

#include <optional>
#include <string>
#include "tile_map/tile_palette.hpp"
#include "tile_map/tile_set.hpp"

class TextureCache;

int tilesAcross(int textureWidth, int tileSize);

std::optional<std::string> whatMovingToWouldBreak(
    const TileSet &saved,
    int savedTextureWidth,
    const TileSet &edited,
    int editedTextureWidth,
    const std::string &paletteName);

void checkTileSetFits(
    const TilePalette &palette,
    const std::string &paletteName,
    int textureWidth,
    int textureHeight);

void warmTileSets(TextureCache &textures, const TilePalettes &tilePalettes);
