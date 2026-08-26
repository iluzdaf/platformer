#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "game/tile_map/tile_map.hpp"
#include "game/npc/npc_data.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_profile.hpp"

class Level
{
public:
    Level(
        const TileMapData &tileMapData,
        const TilePalettes &tilePalettes,
        const std::unordered_map<std::string, NpcData> &npcData);

    const TileMap &getTileMap() const;
    const NavigationGraph &graphFor(const std::string &npcType) const;

private:
    TileMap tileMap;
    std::vector<NavigationGraph> graphs;
    std::unordered_map<std::string, size_t> graphByNpcType;
};
