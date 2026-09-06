#pragma once

#include <map>
#include <string>
#include <utility>
#include "assets/asset_paths.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette_data.hpp"

inline TilePaletteData paletteOf(std::map<int, TileData> tiles)
{
    TilePaletteData palette;
    palette.tileSet.texture = std::string(assets::TileSetTexture);
    palette.tiles = std::move(tiles);
    return palette;
}

inline const TilePaletteData &aPaletteWithASolidTile()
{
    static const TilePaletteData palette = []
    {
        TileData solid;
        solid.solid = solid.grippable = true;
        return paletteOf({{0, TileData{}}, {1, solid}});
    }();
    return palette;
}

inline TilePalettes theOnlyPalette(const TilePaletteData &palette)
{
    return {{"default", palette}};
}

inline constexpr int SpikeTileIndex = 2;

inline TilePaletteData aPaletteWithSpikes()
{
    TilePaletteData palette = aPaletteWithASolidTile();
    TileData spikes;
    spikes.deadly = true;
    palette.tiles[SpikeTileIndex] = spikes;
    return palette;
}
