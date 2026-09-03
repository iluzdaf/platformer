#pragma once

#include <string>
#include <cstddef>
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
#include "pickups/pickup.hpp"
#include "pickups/pickup_data.hpp"
#include <memory>

class Npc;
struct AABB;

class Level
{
public:
    Level(
        const LevelData &levelData,
        const TilePalettes &tilePalettes,
        const PlayerData &playerData,
        const std::map<std::string, NpcData> &npcData,
        const std::map<std::string, PickupData> &pickupData);
    ~Level();

    const TileMap &getTileMap() const;
    TileMap &getTileMap();

    const NavigationGraph &graphFor(const NavigationProfile &profile) const;
    const NavigationGraph &graphForNpc(const NpcSpawnData &spawn) const;
    const std::vector<NamedNavigationGraph> &getGraphs() const;

    std::optional<std::pair<glm::vec2, glm::vec2>> patrolFor(const NpcSpawnData &spawn) const;
    void rebuildGraphs();

    glm::ivec2 getPlayerStartTile() const;
    const std::string &getNextLevel() const;
    const std::vector<NpcSpawnData> &getNpcSpawns() const;
    const std::vector<PickupSpawnData> &getPickupSpawns() const;

    const std::vector<std::unique_ptr<Npc>> &getNpcs() const;
    const std::vector<Pickup> &getPickups() const;

    void rebuildNpcs(const std::map<std::string, NpcData> &npcData);
    void rebuildPickups(const std::map<std::string, PickupData> &pickupData);

    void preFixedUpdate();
    void fixedUpdate(float deltaTime, const glm::vec2 &playerPosition);
    void postFixedUpdate();
    void update(float deltaTime);
    std::vector<Pickup> takePickupsTouching(const AABB &reach);
    void addNpc(const NpcSpawnData &spawn);
    void removeNpc(std::size_t index);
    void setNpcSpawnTile(std::size_t index, glm::ivec2 tilePosition);
    void setNpcPatrol(std::size_t index, PatrolData patrol);
    void clearNpcPatrol(std::size_t index);
    std::optional<PatrolData> runBeneathNpc(std::size_t index) const;
    void setPlayerStartTile(glm::ivec2 tilePosition);
    void setNextLevel(const std::string &levelPath);
    LevelData toLevelData() const;

private:
    TileMap tileMap;
    glm::ivec2 playerStartTilePosition = glm::ivec2(0, 0);
    std::string nextLevel;
    std::vector<NpcSpawnData> npcSpawns;
    std::vector<PickupSpawnData> pickupSpawns;
    std::vector<std::unique_ptr<Npc>> npcs;
    std::vector<Pickup> pickups;
    std::vector<NamedNavigationGraph> graphs;
    std::map<std::string, NavigationProfile> npcProfiles;

    void addGraphFor(const std::string &name, const NavigationProfile &profile);
};
