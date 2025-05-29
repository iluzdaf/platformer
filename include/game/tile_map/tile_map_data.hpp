#pragma once
#include <unordered_map>
#include <vector>
#include <optional>
#include "game/tile_map/tile_data.hpp"

struct TileMapData
{
    int size = 16;
    std::optional<int> width;
    std::optional<int> height;
    std::optional<std::vector<std::vector<int>>> indices;
    std::unordered_map<int, TileData> tileData;
};