#pragma once

#include <map>
#include <string>
#include <utility>
#include "assets/asset_paths.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette_data.hpp"

inline constexpr int EmptyTile = 0;
inline constexpr int SolidTile = 1;
inline constexpr int SpikeTile = 2;
inline constexpr int SlipperyTile = 3;

inline TilePaletteData paletteOf(std::map<int, TileData> tiles)
{
    TilePaletteData palette;
    palette.tileSet.texture.path = std::string(assets::TileSetTexture);
    palette.tiles = std::move(tiles);
    return palette;
}

inline const TilePaletteData &aPaletteWithASolidTile()
{
    static const TilePaletteData palette = []
    {
        TileData solid;
        solid.solid = solid.grippable = true;
        return paletteOf({{EmptyTile, TileData{}}, {SolidTile, solid}});
    }();
    return palette;
}

inline TilePalettes theOnlyPalette(const TilePaletteData &palette)
{
    return {{"default", palette}};
}

inline TilePaletteData aPaletteWithSpikes()
{
    TilePaletteData palette = aPaletteWithASolidTile();
    TileData spikes;
    spikes.deadly = true;
    palette.tiles[SpikeTile] = spikes;
    return palette;
}

inline TilePaletteData aPaletteWithSlipperyTiles()
{
    TilePaletteData palette = aPaletteWithASolidTile();
    TileData slippery;
    slippery.solid = true;
    palette.tiles[SlipperyTile] = slippery;
    return palette;
}
