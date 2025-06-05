#pragma once

#include "game/tile_map/tile_map.hpp"
#include "game/tile_map/tile_map_data.hpp"

inline TileMap setupTileMap(
    int width = 10,
    int height = 10,
    int tileSize = 16,
    const std::unordered_map<int, TileData> &tileDataMap = {})
{
    TileMapData tileMapData;
    tileMapData.size = tileSize;
    tileMapData.width = width;
    tileMapData.height = height;
    tileMapData.tileData = tileDataMap;
    return TileMap(tileMapData);
}