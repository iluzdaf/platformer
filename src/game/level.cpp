#include <cstddef>
#include <optional>
#include <stdexcept>
#include <glaze/glaze.hpp>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include "game/level.hpp"
#include "game/beat_between.hpp"
#include "game/level_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/named_navigation_graph.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "navigation/navigation_graph_builder.hpp"
#include "navigation/navigation_path.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "player/player_data.hpp"
#include "npc/npc_data.hpp"
#include "tile_map/tile.hpp"
#include "npc/npc.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup.hpp"
#include "pickups/pickup_data.hpp"
#include "pickups/collecting.hpp"
#include "game/catalogue.hpp"
#include "actor/actor.hpp"
#include <memory>
#include <set>

Level::Level(
    const LevelData &levelData,
    const TilePalettes &tilePalettes,
    const PlayerData &playerData,
    const std::map<std::string, NpcData> &npcData,
    const std::map<std::string, PickupData> &pickupData)
    : tileMap(levelData.tileMapData, tilePalettes)
{
    playerStart = levelData.playerStart;
    glm::ivec2 startsOn = tileMap.tileUnderFeet(playerStart);
    if (!tileMap.validTilePosition(startsOn))
        throw std::runtime_error("playerStart is out of bounds");

    const Tile &startTile = tileMap.getTileAtTilePosition(startsOn);
    if (startTile.isSolid())
        throw std::runtime_error("Player start position is on a solid tile");
    if (startTile.isDeadly())
        throw std::runtime_error("Player start position is on a spike tile");
    if (startTile.isPortal())
        throw std::runtime_error("Player start position is on a portal tile");

    nextLevel = levelData.nextLevel;
    if (nextLevel.empty())
        throw std::runtime_error("nextLevel must not be empty");

    for (const auto &npc : levelData.npcs)
    {
        glm::ivec2 standsOn = tileMap.tileUnderFeet(npc.position);
        if (!tileMap.validTilePosition(standsOn))
            throw std::runtime_error("Npc start position is out of bounds");
        if (tileMap.getTileAtTilePosition(standsOn).isSolid())
            throw std::runtime_error("Npc start position is on a solid tile");
    }

    addGraphFor("player", buildNavigationProfile(playerData.actorData));

    std::set<std::string> npcTypes;
    for (const auto &[type, data] : npcData)
    {
        npcTypes.insert(type);
        addGraphFor(type, buildNavigationProfile(data.actorData));
    }

    for (const NpcSpawnData &spawn : levelData.npcs)
        if (!npcTypes.contains(spawn.type))
            throw std::runtime_error("Unknown npc \"" + spawn.type + "\"");

    for (const NpcSpawnData &spawn : levelData.npcs)
        npcs.push_back(std::make_unique<Npc>(spawn, oneNamed(npcData, "npc", spawn.type)));

    for (const PickupSpawnData &spawn : levelData.pickups)
    {
        const PickupData &kind = oneNamed(pickupData, "pickup", spawn.type);
        pickups.push_back(Pickup(kind, spawn.position - kind.size * 0.5f));
    }
}

void Level::addGraphFor(const std::string &name, const NavigationProfile &profile)
{
    for (NamedNavigationGraph &existing : graphs)
        if (existing.profile == profile)
        {
            if (existing.name != name && existing.name.find(", " + name) == std::string::npos &&
                !existing.name.starts_with(name + ","))
                existing.name += ", " + name;

            return;
        }

    graphs.push_back({name, profile, buildNavigationGraph(tileMap, profile)});
}

const TileMap &Level::getTileMap() const
{
    return tileMap;
}

const std::vector<NamedNavigationGraph> &Level::getGraphs() const
{
    return graphs;
}

const NavigationGraph &Level::graphFor(const NavigationProfile &profile) const
{
    for (const NamedNavigationGraph &named : graphs)
        if (named.profile == profile)
            return named.graph;

    throw std::runtime_error("This level has no navigation graph for that actor");
}

std::optional<PatrolData> Level::runBeneath(const NavigationProfile &profile, glm::vec2 position)
    const
{
    const NavigationGraph &graph = graphFor(profile);
    std::optional<int> standing = nearestNodeTo(graph, position);
    if (!standing)
        return std::nullopt;

    int leftmost = *standing, rightmost = *standing;
    for (int id : walkableFrom(graph, *standing))
    {
        if (graph.getNode(id).position.x < graph.getNode(leftmost).position.x)
            leftmost = id;

        if (graph.getNode(id).position.x > graph.getNode(rightmost).position.x)
            rightmost = id;
    }

    return beatBetween(
        tileMap,
        tileMap.tileStoodOnAt(graph.getNode(leftmost).position),
        tileMap.tileStoodOnAt(graph.getNode(rightmost).position));
}

glm::vec2 Level::getPlayerStart() const
{
    return playerStart;
}

const std::string &Level::getNextLevel() const
{
    return nextLevel;
}

Level::~Level() = default;

const std::vector<std::unique_ptr<Npc>> &Level::getNpcs() const
{
    return npcs;
}

const std::vector<Pickup> &Level::getPickups() const
{
    return pickups;
}

void Level::preFixedUpdate()
{
    for (const std::unique_ptr<Npc> &npc : npcs)
        npc->preFixedUpdate();
}

void Level::fixedUpdate(float deltaTime, const glm::vec2 &playerPosition)
{
    for (const std::unique_ptr<Npc> &npc : npcs)
        npc->fixedUpdate(deltaTime, *this, playerPosition);
}

void Level::postFixedUpdate()
{
    for (const std::unique_ptr<Npc> &npc : npcs)
        npc->postFixedUpdate();
}

void Level::update(float deltaTime)
{
    tileMap.update(deltaTime);

    for (Pickup &pickup : pickups)
        pickup.update(deltaTime);
}

std::vector<Pickup> Level::takePickupsTouching(const AABB &reach)
{
    return takeWhatTouches(pickups, reach);
}
