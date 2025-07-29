#pragma once
#include <unordered_map>
#include <vector>
#include <optional>
#include "game/tile_map/tile_data.hpp"
#include "serialization/glm_ivec2_meta.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_edge.hpp"

struct TileMapData
{
    int size = 16;
    std::optional<int> width;
    std::optional<int> height;
    std::optional<std::vector<std::vector<int>>> indices;
    std::unordered_map<int, TileData> tileData;
    glm::ivec2 playerStartTilePosition = glm::ivec2(0, 0);
    std::string nextLevel = "../assets/levels/level1.json";
    std::vector<NavigationNode> navigationNodes;
    std::vector<NavigationEdge> navigationEdges;
};