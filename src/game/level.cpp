#include <cstddef>
#include <optional>
#include <stdexcept>
#include <glaze/glaze.hpp>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/named_navigation_graph.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "navigation/navigation_graph_builder.hpp"
#include "navigation/navigation_path.hpp"
#include "tile_map/tile_palette.hpp"
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

namespace
{
}

Level::Level(
    const LevelData &levelData,
    const TilePalettes &tilePalettes,
    const PlayerData &playerData,
    const std::map<std::string, NpcData> &npcData,
    const std::map<std::string, PickupData> &pickupData)
    : tileMap(levelData.tileMapData, tilePalettes)
{
    playerStartTilePosition = levelData.playerStartTilePosition;
    if (!tileMap.validTilePosition(playerStartTilePosition))
        throw std::runtime_error("playerStartTilePosition is out of bounds");

    const Tile &startTile = tileMap.getTileAtTilePosition(playerStartTilePosition);
    if (startTile.isSolid())
        throw std::runtime_error("Player start position is on a solid tile");
    if (startTile.isDeadly())
        throw std::runtime_error("Player start position is on a spike tile");
    if (startTile.isPortal())
        throw std::runtime_error("Player start position is on a portal tile");

    nextLevel = levelData.nextLevel;
    if (nextLevel.empty())
        throw std::runtime_error("nextLevel must not be empty");

    pickupSpawns = levelData.pickups;
    for (const auto &npc : levelData.npcs)
    {
        if (!tileMap.validTilePosition(npc.tilePosition))
            throw std::runtime_error("Npc start position is out of bounds");
        if (tileMap.getTileAtTilePosition(npc.tilePosition).isSolid())
            throw std::runtime_error("Npc start position is on a solid tile");
    }

    addGraphFor("player", buildNavigationProfile(playerData.actorData));

    npcProfiles.clear();
    for (const auto &[type, data] : npcData)
    {
        npcProfiles.emplace(type, buildNavigationProfile(data.actorData));
        addGraphFor(type, npcProfiles.at(type));
    }

    for (const NpcSpawnData &spawn : levelData.npcs)
        if (!npcProfiles.contains(spawn.type))
            throw std::runtime_error("Unknown npc \"" + spawn.type + "\"");

    for (const NpcSpawnData &spawn : levelData.npcs)
        npcs.push_back(
            std::make_unique<Npc>(
                spawn,
                oneNamed(npcData, "npc", spawn.type),
                feetOnTile(spawn.tilePosition),
                patrolFor(spawn)));

    rebuildPickups(pickupData);
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

TileMap &Level::getTileMap()
{
    return tileMap;
}

std::optional<std::pair<glm::vec2, glm::vec2>> Level::patrolFor(const NpcSpawnData &spawn) const
{
    if (!spawn.patrol)
        return std::nullopt;

    return patrolBetween(*spawn.patrol);
}

std::pair<glm::vec2, glm::vec2> Level::patrolBetween(const PatrolData &patrol) const
{
    glm::vec2 from = feetOnTile(patrol.from);
    glm::vec2 to = feetOnTile(patrol.to);
    float outwards = static_cast<float>(tileMap.getTileSize()) * 0.5f;

    if (from.x <= to.x)
    {
        from.x -= outwards;
        to.x += outwards;
    }
    else
    {
        from.x += outwards;
        to.x -= outwards;
    }

    return std::pair(from, to);
}

glm::vec2 Level::feetOnTile(glm::ivec2 tilePosition) const
{
    if (!tileMap.validTilePosition(tilePosition))
        throw std::runtime_error("Tile coordinates out of bounds");

    return tileMap.feetOnTile(tilePosition);
}

const std::vector<NamedNavigationGraph> &Level::getGraphs() const
{
    return graphs;
}

void Level::rebuildGraphs()
{
    for (NamedNavigationGraph &named : graphs)
        named.graph = buildNavigationGraph(tileMap, named.profile);
}

const NavigationGraph &Level::graphFor(const NavigationProfile &profile) const
{
    for (const NamedNavigationGraph &named : graphs)
        if (named.profile == profile)
            return named.graph;

    throw std::runtime_error("This level has no navigation graph for that actor");
}

const NavigationGraph &Level::graphFor(const std::string &npcType) const
{
    auto profile = npcProfiles.find(npcType);
    if (profile == npcProfiles.end())
        throw std::runtime_error("Unknown npc \"" + npcType + "\"");

    return graphFor(profile->second);
}

std::optional<PatrolData> Level::runBeneathNpc(std::size_t index) const
{
    const NpcSpawnData &spawn = getNpc(index).getSpawn();
    const NavigationGraph &graph = graphFor(spawn.type);
    std::optional<int> standing = nearestNodeTo(graph, tileMap.feetOnTile(spawn.tilePosition));
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

    return PatrolData{
        tileMap.tileStoodOnAt(graph.getNode(leftmost).position),
        tileMap.tileStoodOnAt(graph.getNode(rightmost).position)};
}

glm::ivec2 Level::getPlayerStartTile() const
{
    return playerStartTilePosition;
}

const std::string &Level::getNextLevel() const
{
    return nextLevel;
}

Level::~Level() = default;

void Level::rebuildNpcs(const std::map<std::string, NpcData> &npcData)
{
    std::vector<NpcSpawnData> placed;
    placed.reserve(npcs.size());
    for (const std::unique_ptr<Npc> &npc : npcs)
        placed.push_back(npc->getSpawn());

    npcs.clear();
    for (const NpcSpawnData &spawn : placed)
        npcs.push_back(
            std::make_unique<Npc>(
                spawn,
                oneNamed(npcData, "npc", spawn.type),
                feetOnTile(spawn.tilePosition),
                patrolFor(spawn)));
}

void Level::rebuildPickups(const std::map<std::string, PickupData> &pickupData)
{
    pickups.clear();

    for (const PickupSpawnData &spawn : pickupSpawns)
    {
        const PickupData &kind = oneNamed(pickupData, "pickup", spawn.type);
        pickups.push_back(
            Pickup(kind, tileMap.middleOfTile(spawn.tilePosition) - kind.size * 0.5f));
    }
}

const std::vector<std::unique_ptr<Npc>> &Level::getNpcs() const
{
    return npcs;
}

Npc &Level::getNpc(std::size_t index)
{
    return const_cast<Npc &>(std::as_const(*this).getNpc(index));
}

const Npc &Level::getNpc(std::size_t index) const
{
    if (index >= npcs.size())
        throw std::runtime_error(
            "This level has no npc " + std::to_string(index) + ", it has " +
            std::to_string(npcs.size()));

    return *npcs[index];
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

const std::vector<PickupSpawnData> &Level::getPickupSpawns() const
{
    return pickupSpawns;
}

void Level::addNpc(const NpcSpawnData &spawn, const NpcData &npcData)
{
    if (!npcProfiles.contains(spawn.type))
        throw std::runtime_error("Unknown npc \"" + spawn.type + "\"");

    npcs.push_back(
        std::make_unique<Npc>(spawn, npcData, feetOnTile(spawn.tilePosition), patrolFor(spawn)));
}

void Level::removeNpc(std::size_t index)
{
    const Npc &going = getNpc(index);
    std::erase_if(npcs, [&going](const std::unique_ptr<Npc> &npc) { return npc.get() == &going; });
}

void Level::setPlayerStartTile(glm::ivec2 tilePosition)
{
    if (!tileMap.validTilePosition(tilePosition))
        throw std::runtime_error("Tile coordinates out of bounds");

    playerStartTilePosition = tilePosition;
}

void Level::setNextLevel(const std::string &levelPath)
{
    if (levelPath.empty())
        throw std::runtime_error("nextLevel must not be empty");

    nextLevel = levelPath;
}

LevelData Level::toLevelData() const
{
    LevelData levelData;
    levelData.tileMapData = tileMap.toTileMapData();
    levelData.playerStartTilePosition = playerStartTilePosition;
    levelData.nextLevel = nextLevel;
    for (const std::unique_ptr<Npc> &npc : npcs)
        levelData.npcs.push_back(npc->getSpawn());

    levelData.pickups = pickupSpawns;
    return levelData;
}
