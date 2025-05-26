#pragma once
#include "game/tile_data.hpp"
#include <unordered_map>
#include <vector>
#include <optional>

struct TileMapData
{
    int size;
    std::optional<int> width;
    std::optional<int> height;
    std::optional<std::vector<std::vector<int>>> indices;
    std::unordered_map<int, TileData> data;
};