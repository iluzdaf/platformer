#pragma once

#include <map>
#include <glaze/glaze.hpp>
#include "tile_map/tile_map.hpp"
#include "game/level_data.hpp"
#include "tile_map/tile_map_data.hpp"
#include "game/game_data.hpp"
#include "test_helpers/asset_path.hpp"

inline const TilePalette &getDefaultTileDataMap()
{
    static const TilePalette map = []
    {
        TilePalette palette;
        palette[0] = TileData{};
        TileData solid;
        solid.solid = solid.grippable = true;
        palette[1] = solid;
        return palette;
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
    TileMapData tileMapData;
    tileMapData.size = tileSize;
    tileMapData.width = width;
    tileMapData.height = height;
    return TileMap(tileMapData, palettesFrom(palette));
}

inline TileMap tilesOfLevel(const std::string &jsonFilePath)
{
    LevelData levelData;
    auto error = glz::read_file_json(levelData, jsonFilePath, std::string{});
    if (error)
        throw std::runtime_error("Failed to read " + jsonFilePath);

    return TileMap(levelData.tileMapData, shippedPalettes());
}
