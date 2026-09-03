#pragma once

#include <string>
#include <cstddef>
#include <map>
#include <set>
#include <vector>
#include <optional>
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

    const NavigationGraph &graphFor(const NavigationProfile &profile) const;
    const std::vector<NamedNavigationGraph> &getGraphs() const;

    glm::vec2 getPlayerStart() const;
    const std::string &getNextLevel() const;
    const std::vector<PickupSpawnData> &getPickupSpawns() const;

    const std::vector<std::unique_ptr<Npc>> &getNpcs() const;
    const std::vector<Pickup> &getPickups() const;

    void preFixedUpdate();
    void fixedUpdate(float deltaTime, const glm::vec2 &playerPosition);
    void postFixedUpdate();
    void update(float deltaTime);
    std::vector<Pickup> takePickupsTouching(const AABB &reach);
    std::optional<PatrolData> runBeneath(const NavigationProfile &profile, glm::vec2 position)
        const;

private:
    TileMap tileMap;
    glm::vec2 playerStart = glm::vec2(0.0f);
    std::string nextLevel;
    std::vector<PickupSpawnData> pickupSpawns;
    std::vector<std::unique_ptr<Npc>> npcs;
    std::vector<Pickup> pickups;
    std::vector<NamedNavigationGraph> graphs;
    std::set<std::string> npcTypes;

    void addGraphFor(const std::string &name, const NavigationProfile &profile);
    void rebuildPickups(const std::map<std::string, PickupData> &pickupData);
};
