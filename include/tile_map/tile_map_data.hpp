#pragma once

#include <string>
#include <vector>

struct TileMapData
{
    std::vector<std::vector<int>> indices;
    std::string tilePalette;
};
