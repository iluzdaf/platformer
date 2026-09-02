#include <stdexcept>
#include <string>
#include "rendering/tile_set_textures.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/texture_cache.hpp"
#include "tile_map/tile_palette.hpp"
#include "tile_map/tile_set.hpp"

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
    const TileSet &tileSet = palette.tileSet;
    if (tileSet.texture.empty())
        throw std::runtime_error("No tile set texture is named" + named(paletteName));

    if (tileSet.tileSize <= 0)
        throw std::runtime_error("A tile set cell must be wider than 0" + named(paletteName));

    int cells = tilesInSheet(textureWidth, textureHeight, tileSet.tileSize);
    if (cells <= 0)
        throw std::runtime_error(
            "Tile set \"" + tileSet.texture + "\" holds no whole tiles at " +
            std::to_string(tileSet.tileSize) + " across" + named(paletteName));
    for (const auto &[tileIndex, tileData] : palette.tiles)
        if (tileIndex >= cells)
            throw std::runtime_error(
                "Tile " + std::to_string(tileIndex) + " is past the " + std::to_string(cells) +
                " tiles of \"" + tileSet.texture + "\"" + named(paletteName));

    if (palette.scoreTile && (palette.scoreTile->value < 0 || palette.scoreTile->value >= cells))
        throw std::runtime_error(
            "The score is counted on tile " + std::to_string(palette.scoreTile->value) +
            ", which is not one of the " + std::to_string(cells) + " tiles of \"" +
            tileSet.texture + "\"" + named(paletteName));
}

void warmTileSets(TextureCache &textures, const TilePalettes &tilePalettes)
{
    for (const auto &[paletteName, palette] : tilePalettes)
    {
        if (palette.tileSet.texture.empty())
            throw std::runtime_error("No tile set texture is named" + named(paletteName));

        textures.warm(palette.tileSet.texture);
        const Texture2D &texture = textures.get(palette.tileSet.texture);

        checkTileSetFits(
            palette,
            paletteName,
            static_cast<int>(texture.getWidth()),
            static_cast<int>(texture.getHeight()));
    }
}
