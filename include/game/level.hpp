#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "game/tile_map/tile_map.hpp"
#include "game/npc/npc_data.hpp"
#include "game/player/player_data.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_profile.hpp"

class Level
{
public:
    Level(
        const std::string &jsonFilePath,
        const TilePalettes &tilePalettes,
        const PlayerData &playerData,
        const std::unordered_map<std::string, NpcData> &npcData);
    Level(
        const TileMapData &tileMapData,
        const TilePalettes &tilePalettes,
        const PlayerData &playerData,
        const std::unordered_map<std::string, NpcData> &npcData);

    const TileMap &getTileMap() const;
    TileMap &getTileMap();
    const std::vector<NavigationGraph> &getGraphs() const;
    void rebuildGraphs();
    const NavigationGraph &graphFor(const NavigationProfile &profile) const;

private:
    TileMap tileMap;
    std::vector<NavigationProfile> profiles;
    std::vector<NavigationGraph> graphs;

    void addGraphFor(const NavigationProfile &profile);
    void buildGraphs(
        const PlayerData &playerData,
        const std::unordered_map<std::string, NpcData> &npcData);
};
