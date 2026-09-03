#pragma once

#include <utility>
#include <map>
#include <glaze/glaze.hpp>
#include <string>
#include <stdexcept>
#include "tile_map/tile_map.hpp"
#include "game/level_data.hpp"
#include "tile_map/tile_map_data.hpp"
#include "game/game_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "tile_map/tile_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "assets/asset_paths.hpp"

inline TilePalette paletteOf(std::map<int, TileData> tiles)
{
    TilePalette palette;
    palette.tileSet.texture = std::string(assets::TileSetTexture);
    palette.tiles = std::move(tiles);
    return palette;
}

inline const TilePalette &getDefaultTileDataMap()
{
    static const TilePalette map = []
    {
        TileData solid;
        solid.solid = solid.grippable = true;
        return paletteOf({{0, TileData{}}, {1, solid}});
    }();
    return map;
}

inline const TilePalettes &shippedPalettes()
{
    static const TilePalettes palettes = [] { return loadGameData().tilePalettes; }();
    return palettes;
}

inline const std::map<std::string, NpcData> &shippedNpcData()
{
    static const std::map<std::string, NpcData> npcData = [] { return loadGameData().npcData; }();
    return npcData;
}

inline const std::map<std::string, PickupData> &shippedPickupData()
{
    static const std::map<std::string, PickupData> pickupData = []
    { return loadGameData().pickupData; }();
    return pickupData;
}

inline int aTileIn(const TilePalette &palette, bool TileData::*what)
{
    for (const auto &[tileIndex, tileData] : palette.tiles)
        if (tileData.*what)
            return tileIndex;

    throw std::runtime_error("no tile in this palette is what the test needs");
}

inline int aSolidTileIn(const TilePalette &palette)
{
    return aTileIn(palette, &TileData::solid);
}

inline int aDeadlyTileIn(const TilePalette &palette)
{
    return aTileIn(palette, &TileData::deadly);
}

inline TilePalettes palettesFrom(const TilePalette &palette)
{
    return {{"default", palette}};
}

inline TileMap setupTileMap(
    int width = 10,
    int height = 10,
    int tileSize = 16,
    const TilePalette &palette = getDefaultTileDataMap())
{
    TilePalette sized = palette;
    sized.tileSet.cellSize = glm::ivec2(tileSize);

    TileMapData tileMapData;
    tileMapData.width = width;
    tileMapData.height = height;
    return TileMap(tileMapData, palettesFrom(sized));
}

inline TileMap tilesOfLevel(const std::string &jsonFilePath)
{
    LevelData levelData;
    auto error = glz::read_file_json(levelData, jsonFilePath, std::string{});
    if (error)
        throw std::runtime_error("Failed to read " + jsonFilePath);

    return TileMap(levelData.tileMapData, shippedPalettes());
}
