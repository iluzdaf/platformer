#include <cmath>
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <glaze/glaze.hpp>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include "game/level.hpp"
#include "assets/asset_paths.hpp"
#include "serialization/json_format.hpp"
#include "game/level_data.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/named_navigation_graph.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "navigation/navigation_graph_builder.hpp"
#include "navigation/navigation_path.hpp"
#include "tile_map/tile_palette.hpp"
#include "player/player_data.hpp"
#include "npc/npc_data.hpp"
#include "tile_map/tile.hpp"

namespace
{

    std::string withPaddedGrid(const std::string &json)
    {
        const std::string key = "\"indices\":[";
        size_t start = json.find(key);
        if (start == std::string::npos)
            return json;

        size_t cursor = start + key.size();
        std::vector<std::vector<std::string>> rows;

        while (cursor < json.size() && json[cursor] == '[')
        {
            size_t end = json.find(']', cursor);
            if (end == std::string::npos)
                return json;

            std::vector<std::string> cells;
            for (size_t cell = cursor + 1; cell < end;)
            {
                size_t comma = json.find(',', cell);
                if (comma == std::string::npos || comma > end)
                    comma = end;

                cells.push_back(json.substr(cell, comma - cell));
                cell = comma + 1;
            }
            rows.push_back(std::move(cells));

            cursor = end + 1;
            if (cursor < json.size() && json[cursor] == ',')
                ++cursor;
        }

        size_t width = 0;
        for (const auto &row : rows)
            for (const auto &cell : row)
                width = std::max(width, cell.size());

        std::string out = json.substr(0, start + key.size());
        for (size_t row = 0; row < rows.size(); ++row)
        {
            out += "[";
            for (size_t cell = 0; cell < rows[row].size(); ++cell)
            {
                if (cell > 0)
                    out += ",";

                out += std::string(width - rows[row][cell].size(), ' ') + rows[row][cell];
            }
            out += "]";

            if (row + 1 < rows.size())
                out += ",";
        }

        return out + json.substr(cursor);
    }

    LevelData readLevelData(const std::string &jsonFilePath)
    {
        LevelData levelData;
        auto error = glz::read_file_json(levelData, jsonFilePath, std::string{});
        if (error)
            throw std::runtime_error("Failed to read level json file " + jsonFilePath);

        return levelData;
    }
}

Level::Level(
    const std::string &levelPath,
    const TilePalettes &tilePalettes,
    const PlayerData &playerData,
    const std::map<std::string, NpcData> &npcData)
    : Level(readLevelData(assets::pathTo(levelPath)), tilePalettes, playerData, npcData, levelPath)
{
}

Level::Level(
    const LevelData &levelData,
    const TilePalettes &tilePalettes,
    const PlayerData &playerData,
    const std::map<std::string, NpcData> &npcData)
    : Level(levelData, tilePalettes, playerData, npcData, "levels/new_level.json")
{
}

Level::Level(
    const LevelData &levelData,
    const TilePalettes &tilePalettes,
    const PlayerData &playerData,
    const std::map<std::string, NpcData> &npcData,
    const std::string &levelPath)
    : tileMap(levelData.tileMapData, tilePalettes), path(levelPath)
{
    initFrom(levelData, playerData, npcData);
}

void Level::initFrom(
    const LevelData &levelData,
    const PlayerData &playerData,
    const std::map<std::string, NpcData> &npcData)
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

    npcs = levelData.npcs;
    for (const auto &npc : npcs)
    {
        if (!tileMap.validTilePosition(npc.tilePosition))
            throw std::runtime_error("Npc start position is out of bounds");
        if (tileMap.getTileAtTilePosition(npc.tilePosition).isSolid())
            throw std::runtime_error("Npc start position is on a solid tile");
    }

    addGraphFor("player", buildNavigationProfile(playerData.actorData));

    npcProfiles.clear();
    for (const auto &[type, data] : npcData)
        npcProfiles.emplace(type, buildNavigationProfile(data.actorData));

    for (const NpcSpawnData &spawn : npcs)
    {
        if (!npcProfiles.contains(spawn.type))
            throw std::runtime_error("Unknown npc \"" + spawn.type + "\" in " + path);

        addGraphFor(spawn.type, npcProfiles.at(spawn.type));
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

TileMap &Level::getTileMap()
{
    return tileMap;
}

std::optional<std::pair<glm::vec2, glm::vec2>> Level::patrolFor(const NpcSpawnData &spawn) const
{
    if (!spawn.patrol)
        return std::nullopt;

    return std::pair(
        tileMap.tileToBottomCenterPosition(spawn.patrol->from),
        tileMap.tileToBottomCenterPosition(spawn.patrol->to));
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

const NavigationGraph &Level::graphForNpc(const NpcSpawnData &spawn) const
{
    auto profile = npcProfiles.find(spawn.type);
    if (profile == npcProfiles.end())
        throw std::runtime_error("Unknown npc \"" + spawn.type + "\"");

    return graphFor(profile->second);
}

glm::ivec2 Level::beatEndAt(const NpcSpawnData &spawn, glm::ivec2 tilePosition) const
{
    if (!tileMap.validTilePosition(tilePosition))
        throw std::runtime_error("Tile coordinates out of bounds");

    const NavigationGraph &graph = graphForNpc(spawn);
    glm::vec2 asked = tileMap.tileToBottomCenterPosition(tilePosition);
    std::optional<int> standing = nodeUnderfoot(graph, asked);
    if (!standing)
        return tilePosition;

    float surface = graph.getNode(*standing).position.y;
    float leftEnd = graph.getNode(*standing).position.x;
    float rightEnd = leftEnd;
    for (int id : walkableFrom(graph, *standing))
    {
        glm::vec2 node = graph.getNode(id).position;
        if (std::abs(node.y - surface) > 1.0f)
            continue;

        leftEnd = std::min(leftEnd, node.x);
        rightEnd = std::max(rightEnd, node.x);
    }

    return tileMap.bottomCenterToTilePosition(
        glm::vec2(std::clamp(asked.x, leftEnd, rightEnd), surface));
}

std::optional<PatrolData> Level::runBeneathNpc(std::size_t index) const
{
    if (index >= npcs.size())
        throw std::runtime_error("Cannot look under an npc the level does not have");

    const NpcSpawnData &spawn = npcs[index];
    const NavigationGraph &graph = graphForNpc(spawn);
    std::optional<int> standing =
        nearestNodeTo(graph, tileMap.tileToBottomCenterPosition(spawn.tilePosition));
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
        tileMap.bottomCenterToTilePosition(graph.getNode(leftmost).position),
        tileMap.bottomCenterToTilePosition(graph.getNode(rightmost).position)};
}

glm::ivec2 Level::getPlayerStartTile() const
{
    return playerStartTilePosition;
}

const std::string &Level::getNextLevel() const
{
    return nextLevel;
}

const std::vector<NpcSpawnData> &Level::getNpcs() const
{
    return npcs;
}

void Level::addNpc(const NpcSpawnData &spawn)
{
    if (!npcProfiles.contains(spawn.type))
        throw std::runtime_error("Unknown npc \"" + spawn.type + "\"");

    addGraphFor(spawn.type, npcProfiles.at(spawn.type));
    npcs.push_back(spawn);
}

void Level::removeNpc(std::size_t index)
{
    if (index >= npcs.size())
        throw std::runtime_error("Cannot remove an npc the level does not have");

    npcs.erase(npcs.begin() + static_cast<std::ptrdiff_t>(index));
}

void Level::setNpcSpawnTile(std::size_t index, glm::ivec2 tilePosition)
{
    if (index >= npcs.size())
        throw std::runtime_error("Cannot move an npc the level does not have");

    if (!tileMap.validTilePosition(tilePosition))
        throw std::runtime_error("Tile coordinates out of bounds");

    npcs[index].tilePosition = tilePosition;
}

void Level::setNpcPatrol(std::size_t index, PatrolData patrol)
{
    if (index >= npcs.size())
        throw std::runtime_error("Cannot give a beat to an npc the level does not have");

    if (!tileMap.validTilePosition(patrol.from) || !tileMap.validTilePosition(patrol.to))
        throw std::runtime_error("Tile coordinates out of bounds");

    npcs[index].patrol = patrol;
}

void Level::clearNpcPatrol(std::size_t index)
{
    if (index >= npcs.size())
        throw std::runtime_error("Cannot clear the beat of an npc the level does not have");

    npcs[index].patrol.reset();
}

const std::string &Level::getPath() const
{
    return path;
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
    levelData.npcs = npcs;
    return levelData;
}

void Level::save() const
{
    LevelData levelData = toLevelData();
    std::string json;
    auto error = glz::write_json(levelData, json);
    if (error)
        throw std::runtime_error("Failed to serialize LevelData to JSON");

    std::ofstream outFile(assets::pathTo(path));
    outFile << withStructureOnLines(withPaddedGrid(json));
    outFile.close();
}
