#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <glaze/glaze.hpp>
#include "game/level.hpp"
#include "assets/asset_paths.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "navigation/navigation_graph_builder.hpp"

namespace
{
    constexpr int NestingOnLines = 2;
    constexpr size_t InlineWidthLimit = 100;

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

    struct Span
    {
        bool holdsContainer = false;
        size_t length = 0;
    };

    Span spanOf(const std::string &json, size_t opening)
    {
        Span span;
        int depth = 0;
        bool inString = false;
        bool escaped = false;

        for (size_t at = opening; at < json.size(); ++at)
        {
            char character = json[at];

            if (escaped)
                escaped = false;
            else if (inString)
            {
                if (character == '\\')
                    escaped = true;
                else if (character == '"')
                    inString = false;
            }
            else if (character == '"')
                inString = true;
            else if (character == '{' || character == '[')
            {
                if (++depth > 1)
                    span.holdsContainer = true;
            }
            else if (character == '}' || character == ']')
            {
                if (--depth == 0)
                {
                    span.length = at - opening + 1;
                    return span;
                }
            }
        }

        return span;
    }

    std::string withStructureOnLines(const std::string &json)
    {
        std::string out;
        std::vector<bool> expanded;
        int depth = 0;
        bool inString = false;
        bool escaped = false;

        for (size_t at = 0; at < json.size(); ++at)
        {
            char character = json[at];

            if (escaped)
            {
                out += character;
                escaped = false;
                continue;
            }

            if (inString)
            {
                out += character;
                if (character == '\\')
                    escaped = true;
                else if (character == '"')
                    inString = false;
                continue;
            }

            if (character == '"')
            {
                out += character;
                inString = true;
            }
            else if (character == '{' || character == '[')
            {
                char closing = character == '{' ? '}' : ']';
                if (at + 1 < json.size() && json[at + 1] == closing)
                {
                    out += character;
                    out += closing;
                    ++at;
                    continue;
                }

                size_t lineStart = out.rfind('\n');
                size_t column =
                    lineStart == std::string::npos ? out.size() : out.size() - lineStart - 1;

                Span span = spanOf(json, at);
                bool nearTop = span.holdsContainer && depth + 1 <= NestingOnLines;
                bool tooLong = column + span.length > InlineWidthLimit;
                bool scalarArray = character == '[' && !span.holdsContainer;

                bool onLines = !scalarArray && (nearTop || tooLong);
                out += character;
                ++depth;
                expanded.push_back(onLines);

                if (onLines)
                    out += "\n" + std::string(4 * depth, ' ');
            }
            else if (character == '}' || character == ']')
            {
                bool onLines = !expanded.empty() && expanded.back();
                if (!expanded.empty())
                    expanded.pop_back();

                --depth;
                if (onLines)
                    out += "\n" + std::string(4 * depth, ' ');

                out += character;
            }
            else if (character == ',')
            {
                out += character;
                if (!expanded.empty() && expanded.back())
                    out += "\n" + std::string(4 * depth, ' ');
            }
            else
            {
                out += character;
            }
        }

        return out;
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
    const std::unordered_map<std::string, NpcData> &npcData)
    : Level(readLevelData(assets::pathTo(levelPath)), tilePalettes, playerData, npcData, levelPath)
{
}

Level::Level(
    const LevelData &levelData,
    const TilePalettes &tilePalettes,
    const PlayerData &playerData,
    const std::unordered_map<std::string, NpcData> &npcData)
    : Level(levelData, tilePalettes, playerData, npcData, "../assets/levels/new_level.json")
{
}

Level::Level(
    const LevelData &levelData,
    const TilePalettes &tilePalettes,
    const PlayerData &playerData,
    const std::unordered_map<std::string, NpcData> &npcData,
    const std::string &levelPath)
    : tileMap(levelData.tileMapData, tilePalettes), path(levelPath)
{
    initFrom(levelData, playerData, npcData);
}

void Level::initFrom(
    const LevelData &levelData,
    const PlayerData &playerData,
    const std::unordered_map<std::string, NpcData> &npcData)
{
    playerStartTilePosition = levelData.playerStartTilePosition;
    if (!tileMap.validTilePosition(playerStartTilePosition))
        throw std::runtime_error("playerStartTilePosition is out of bounds");

    const Tile &startTile = tileMap.getTileAtTilePosition(playerStartTilePosition);
    if (startTile.isSolid())
        throw std::runtime_error("Player start position is on a solid tile");
    if (startTile.isSpikes())
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

    for (const NpcSpawnData &spawn : npcs)
    {
        auto npc = npcData.find(spawn.type);
        if (npc == npcData.end())
            throw std::runtime_error("Unknown npc \"" + spawn.type + "\" in " + path);

        addGraphFor(spawn.type, buildNavigationProfile(npc->second.actorData));
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
