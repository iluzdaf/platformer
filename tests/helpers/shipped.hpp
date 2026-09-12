#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <glaze/glaze.hpp>
#include "game/game_data.hpp"
#include "game/level_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette_data.hpp"

inline const TilePalettes &shippedPalettes()
{
    static const TilePalettes palettes = [] { return loadGameData().tilePalettes; }();
    return palettes;
}

inline const std::string &shippedPaletteName()
{
    return shippedPalettes().begin()->first;
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

inline TileMap tilesOfLevel(const std::string &jsonFilePath)
{
    LevelData levelData;
    auto error = glz::read_file_json(levelData, jsonFilePath, std::string{});
    if (error)
        throw std::runtime_error("Failed to read " + jsonFilePath);

    return TileMap(levelData.tileMapData, shippedPalettes());
}
