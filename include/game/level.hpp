#pragma once

#include <string>
#include <cstddef>
#include <functional>
#include <map>
#include <vector>
#include <optional>
#include <utility>
#include "game/level_data.hpp"
#include "tile_map/tile_map.hpp"
#include "npc/npc_data.hpp"
#include "player/player_data.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/named_navigation_graph.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile_palette.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "game/renames.hpp"
#include "tile_map/tile_map_data.hpp"

void writeLevelData(const LevelData &levelData, const std::string &levelPath);

bool renamePaletteIn(TileMapData &tileMapData, const Renames &renames);
bool renameTypeIn(std::vector<NpcSpawnData> &npcs, const Renames &renames);
bool renameTypeIn(std::vector<PickupSpawnData> &pickups, const Renames &renames);

std::vector<std::string> renameInLevels(
    const std::string &directory,
    const std::function<bool(LevelData &)> &rewrite);

class Level
{
public:
    Level(
        const std::string &levelPath,
        const TilePalettes &tilePalettes,
        const PlayerData &playerData,
        const std::map<std::string, NpcData> &npcData);
    Level(
        const LevelData &levelData,
        const TilePalettes &tilePalettes,
        const PlayerData &playerData,
        const std::map<std::string, NpcData> &npcData);

    const TileMap &getTileMap() const;
    TileMap &getTileMap();

    const NavigationGraph &graphFor(const NavigationProfile &profile) const;
    const NavigationGraph &graphForNpc(const NpcSpawnData &spawn) const;
    const std::vector<NamedNavigationGraph> &getGraphs() const;

    std::optional<std::pair<glm::vec2, glm::vec2>> patrolFor(const NpcSpawnData &spawn) const;
    void rebuildGraphs();

    glm::ivec2 getPlayerStartTile() const;
    const std::string &getNextLevel() const;
    const std::vector<NpcSpawnData> &getNpcs() const;
    const std::vector<PickupSpawnData> &getPickups() const;
    void addNpc(const NpcSpawnData &spawn);
    void removeNpc(std::size_t index);
    void setNpcSpawnTile(std::size_t index, glm::ivec2 tilePosition);
    void setNpcPatrol(std::size_t index, PatrolData patrol);
    void clearNpcPatrol(std::size_t index);
    std::optional<PatrolData> runBeneathNpc(std::size_t index) const;
    const std::string &getPath() const;
    void setPlayerStartTile(glm::ivec2 tilePosition);
    void setNextLevel(const std::string &levelPath);
    LevelData toLevelData() const;
    void save() const;

private:
    Level(
        const LevelData &levelData,
        const TilePalettes &tilePalettes,
        const PlayerData &playerData,
        const std::map<std::string, NpcData> &npcData,
        const std::string &levelPath);

    TileMap tileMap;
    glm::ivec2 playerStartTilePosition = glm::ivec2(0, 0);
    std::string nextLevel;
    std::vector<NpcSpawnData> npcs;
    std::vector<PickupSpawnData> pickups;
    std::string path;
    std::vector<NamedNavigationGraph> graphs;
    std::map<std::string, NavigationProfile> npcProfiles;

    void initFrom(
        const LevelData &levelData,
        const PlayerData &playerData,
        const std::map<std::string, NpcData> &npcData);
    void addGraphFor(const std::string &name, const NavigationProfile &profile);
};
