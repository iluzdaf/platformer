#include <optional>
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

int tilesAcross(int textureWidth, int tileSize)
{
    return tileSize > 0 ? textureWidth / tileSize : 0;
}

std::optional<std::string> whatMovingToWouldBreak(
    const TileSet &saved,
    int savedTextureWidth,
    const TileSet &edited,
    int editedTextureWidth,
    const std::string &paletteName)
{
    if (saved == edited)
        return std::nullopt;

    int was = tilesAcross(savedTextureWidth, saved.tileSize);
    int now = tilesAcross(editedTextureWidth, edited.tileSize);
    if (was == now)
        return std::nullopt;

    return "every tile in every level using \"" + paletteName +
           "\" would move: " + std::to_string(was) + " across becomes " + std::to_string(now);
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

    if (textureWidth != textureHeight)
        throw std::runtime_error(
            "Tile set \"" + tileSet.texture + "\" must be square, and is " +
            std::to_string(textureWidth) + " by " + std::to_string(textureHeight) +
            named(paletteName));

    int across = textureWidth / tileSet.tileSize;
    int cells = across * across;
    for (const auto &[tileIndex, tileData] : palette.tiles)
        if (tileIndex >= cells)
            throw std::runtime_error(
                "Tile " + std::to_string(tileIndex) + " is past the " + std::to_string(cells) +
                " tiles of \"" + tileSet.texture + "\"" + named(paletteName));
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
