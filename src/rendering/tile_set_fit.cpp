#include <stdexcept>
#include <string>
#include "rendering/tile_set_fit.hpp"
#include "tile_map/tile_palette.hpp"
#include "assets/sheet.hpp"

namespace
{
    std::string named(const std::string &paletteName)
    {
        return " for palette \"" + paletteName + "\"";
    }
}

int tilesInSheet(int textureWidth, int textureHeight, int tileSize)
{
    if (tileSize <= 0)
        return 0;

    return (textureWidth / tileSize) * (textureHeight / tileSize);
}

void checkTileSetFits(
    const TilePalette &palette,
    const std::string &paletteName,
    int textureWidth,
    int textureHeight)
{
    const Sheet &tileSet = palette.tileSet;
    if (tileSet.cellSize.x <= 0)
        throw std::runtime_error("A tile set cell must be wider than 0" + named(paletteName));

    int cells = tilesInSheet(textureWidth, textureHeight, tileSet.cellSize.x);
    if (cells <= 0)
        throw std::runtime_error(
            "Tile set \"" + tileSet.texture + "\" holds no whole tiles at " +
            std::to_string(tileSet.cellSize.x) + " across" + named(paletteName));
    for (const auto &[tileIndex, tileData] : palette.tiles)
        if (tileIndex >= cells)
            throw std::runtime_error(
                "Tile " + std::to_string(tileIndex) + " is past the " + std::to_string(cells) +
                " tiles of \"" + tileSet.texture + "\"" + named(paletteName));
}
