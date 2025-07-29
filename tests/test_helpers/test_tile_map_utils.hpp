#pragma once

#include "game/tile_map/tile_map.hpp"
#include "game/tile_map/tile_map_data.hpp"

inline const std::unordered_map<int, TileData> &getDefaultTileDataMap()
{
    static const std::unordered_map<int, TileData> map = {
        {0, TileData{TileKind::Empty, std::nullopt, std::nullopt, std::nullopt, glm::vec2(0.0f), glm::vec2(16.0f)}},
        {1, TileData{TileKind::Solid, std::nullopt, std::nullopt, std::nullopt, glm::vec2(0.0f), glm::vec2(16.0f)}}};
    return map;
}

inline TileMap setupTileMap(
    int width = 10,
    int height = 10,
    int tileSize = 16,
    const std::unordered_map<int, TileData> &tileDataMap = getDefaultTileDataMap())
{
    TileMapData tileMapData;
    tileMapData.size = tileSize;
    tileMapData.width = width;
    tileMapData.height = height;
    tileMapData.tileData = tileDataMap;
    return TileMap(tileMapData);
}