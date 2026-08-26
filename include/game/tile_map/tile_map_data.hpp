#pragma once

#include <string>
#include <vector>
#include <optional>

struct TileMapData
{
    int size = 16;
    std::optional<int> width;
    std::optional<int> height;
    std::optional<std::vector<std::vector<int>>> indices;
    std::string tilePalette = "default";
};
