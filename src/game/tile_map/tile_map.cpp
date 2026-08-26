#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <glaze/glaze.hpp>
#include "game/tile_map/tile_map.hpp"

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
                size_t column = lineStart == std::string::npos ? out.size() : out.size() - lineStart - 1;

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
}

TileMap::TileMap(const std::string &jsonFilePath, const TilePalettes &tilePalettes) : level(jsonFilePath)
{
    if (jsonFilePath.empty())
    {
        throw std::runtime_error("Tile Map jsonFilePath is empty");
    }

    TileMapData tileMapData;
    auto ec = glz::read_file_json(tileMapData, jsonFilePath, std::string{});
    if (ec)
    {
        throw std::runtime_error("Failed to read Tile Map json file");
    }

    initByData(tileMapData, tilePalettes);
}

TileMap::TileMap(const TileMapData &tileMapData, const TilePalettes &tilePalettes)
{
    initByData(tileMapData, tilePalettes);
}

void TileMap::initByData(const TileMapData &tileMapData, const TilePalettes &tilePalettes)
{
    tileSize = tileMapData.size;

    if (tileSize <= 0)
        throw std::runtime_error("tileSize must be greater than 0");

    const bool hasTileIndices = tileMapData.indices.has_value();
    const bool hasExplicitSize = tileMapData.width.has_value() && tileMapData.height.has_value();
    if (hasTileIndices && hasExplicitSize)
        throw std::runtime_error("Cannot specify both tileIndices and width/height explicitly.");

    if (hasTileIndices)
    {
        const auto &indices = tileMapData.indices.value();
        height = static_cast<int>(indices.size());
        width = height > 0 ? static_cast<int>(indices[0].size()) : 0;

        for (int tileY = 0; tileY < height; ++tileY)
        {
            if (static_cast<int>(indices[tileY].size()) != width)
            {
                throw std::runtime_error("Inconsistent row width in tileIndices");
            }
        }

        tileIndices = std::vector<std::vector<int>>(width, std::vector<int>(height, 0));
        for (int tileY = 0; tileY < height; ++tileY)
        {
            for (int tileX = 0; tileX < width; ++tileX)
            {
                tileIndices[tileX][tileY] = indices[tileY][tileX];
            }
        }
    }
    else if (hasExplicitSize)
    {
        height = tileMapData.height.value();
        width = tileMapData.width.value();
        tileIndices = std::vector<std::vector<int>>(width, std::vector<int>(height, 0));
    }
    else
        throw std::runtime_error("Must specify either tileIndices or width/height.");

    if (width == 0 || height == 0)
        throw std::runtime_error("TileMapData has invalid dimensions");

    tilePalette = tileMapData.tilePalette;
    auto palette = tilePalettes.find(tilePalette);
    if (palette == tilePalettes.end())
        throw std::runtime_error("Unknown tile palette \"" + tilePalette + "\"");

    tiles.insert_or_assign(0, Tile(0, TileData(TileKind::Empty)));
    for (const auto &[tileIndex, tileData] : palette->second)
        tiles.insert_or_assign(tileIndex, Tile(tileIndex, tileData));

    playerStartTilePosition = tileMapData.playerStartTilePosition;
    if (playerStartTilePosition.x < 0 || playerStartTilePosition.y < 0 ||
        playerStartTilePosition.x >= width || playerStartTilePosition.y >= height)
        throw std::runtime_error("playerStartTilePosition is out of bounds");

    const Tile &tile = getTileAtTilePosition(playerStartTilePosition);
    if (tile.isSolid())
        throw std::runtime_error("Player start position is on a solid tile");
    if (tile.isSpikes())
        throw std::runtime_error("Player start position is on a spike tile");
    if (tile.isPortal())
        throw std::runtime_error("Player start position is on a portal tile");

    nextLevel = tileMapData.nextLevel;
    if (nextLevel.empty())
        throw std::runtime_error("nextLevel must not be empty");

    npcs = tileMapData.npcs;
    for (const auto &npc : npcs)
    {
        if (!validTilePosition(npc.tilePosition))
            throw std::runtime_error("Npc start position is out of bounds");
        if (getTileAtTilePosition(npc.tilePosition).isSolid())
            throw std::runtime_error("Npc start position is on a solid tile");
    }
}

void TileMap::setTileIndex(glm::ivec2 tilePosition, int tileIndex)
{
    if (!validTilePosition(tilePosition))
        throw std::runtime_error("Tile coordinates out of bounds");

    if (tileIndex < 0)
        throw std::runtime_error("Tile index must be greater or equals to 0");

    tileIndices[tilePosition.x][tilePosition.y] = tileIndex;
}

void TileMap::setTileIndexAt(glm::vec2 worldPosition, int tileIndex)
{
    setTileIndex(worldToTilePosition(worldPosition), tileIndex);
}

bool TileMap::validTilePosition(glm::ivec2 tilePosition) const
{
    return (tilePosition.x >= 0 &&
            tilePosition.x < width &&
            tilePosition.y >= 0 &&
            tilePosition.y < height);
}

int TileMap::tilePositionToTileIndex(glm::ivec2 tilePosition) const
{
    if (!validTilePosition(tilePosition))
        throw std::runtime_error("Tile coordinates out of bounds");

    return tileIndices[tilePosition.x][tilePosition.y];
}

glm::ivec2 TileMap::worldToTilePosition(glm::vec2 worldPosition) const
{
    return glm::ivec2(static_cast<int>(worldPosition.x) / tileSize, static_cast<int>(worldPosition.y) / tileSize);
}

int TileMap::worldPositionToTileIndex(glm::vec2 worldPosition) const
{
    return tilePositionToTileIndex(worldToTilePosition(worldPosition));
}

int TileMap::getWidth() const
{
    return width;
}

int TileMap::getHeight() const
{
    return height;
}

const Tile &TileMap::getTile(int tileIndex) const
{
    auto it = tiles.find(tileIndex);
    if (it == tiles.end())
        throw std::runtime_error("Invalid tile index");
    return it->second;
}

const Tile &TileMap::getTileAtTilePosition(glm::ivec2 tilePosition) const
{
    return getTile(tilePositionToTileIndex(tilePosition));
}

const Tile &TileMap::getTileAtWorldPosition(glm::vec2 worldPosition) const
{
    return getTile(worldPositionToTileIndex(worldPosition));
}

int TileMap::getTileSize() const
{
    return tileSize;
}

void TileMap::update(float deltaTime)
{
    for (auto &[_, tile] : tiles)
        tile.update(deltaTime);
}

int TileMap::getWorldWidth() const
{
    return width * tileSize;
}

int TileMap::getWorldHeight() const
{
    return height * tileSize;
}

glm::vec2 TileMap::getPlayerStartWorldPosition() const
{
    return tileToWorldPosition(playerStartTilePosition);
}

const std::string &TileMap::getNextLevel() const
{
    return nextLevel;
}

std::vector<glm::ivec2> TileMap::worldToTilePositions(glm::vec2 worldPosition, glm::vec2 size) const
{
    std::vector<glm::ivec2> tileCoordinates;
    glm::vec2 worldPositionMax = worldPosition + size;
    float tileSizePixels = static_cast<float>(tileSize);
    int tileMinX = static_cast<int>(floor(worldPosition.x / tileSizePixels));
    int tileMaxX = static_cast<int>(floor(worldPositionMax.x / tileSizePixels));
    int tileMinY = static_cast<int>(floor(worldPosition.y / tileSizePixels));
    int tileMaxY = static_cast<int>(floor(worldPositionMax.y / tileSizePixels));
    for (int tileY = tileMinY; tileY <= tileMaxY; ++tileY)
    {
        for (int tileX = tileMinX; tileX <= tileMaxX; ++tileX)
        {
            glm::ivec2 tilePosition(tileX, tileY);
            if (!validTilePosition(tilePosition))
                continue;
            tileCoordinates.push_back(tilePosition);
        }
    }

    return tileCoordinates;
}

glm::vec2 TileMap::tileToWorldPosition(glm::ivec2 tilePosition) const
{
    return glm::vec2(tilePosition.x * tileSize, tilePosition.y * tileSize);
}

const std::unordered_map<int, Tile> &TileMap::getTiles() const
{
    return tiles;
}

TileMapData TileMap::toTileMapData() const
{
    TileMapData data;
    data.size = tileSize;
    data.nextLevel = nextLevel;
    data.playerStartTilePosition = playerStartTilePosition;
    data.indices = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            (*data.indices)[y][x] = tileIndices[x][y];
    data.tilePalette = tilePalette;
    data.npcs = npcs;
    return data;
}

void TileMap::save() const
{
    TileMapData data = toTileMapData();
    std::string json;
    auto result = glz::write_json(data, json);
    if (result)
        throw std::runtime_error("Failed to serialize TileMapData to JSON");
    std::ofstream outFile(level);
    outFile << withStructureOnLines(withPaddedGrid(json));
    outFile.close();
}

void TileMap::setPlayerStartTile(glm::ivec2 tilePosition)
{
    playerStartTilePosition = tilePosition;
}

const std::string &TileMap::getLevel() const
{
    return level;
}

bool TileMap::probeSolidTiles(
    const AABB &probeAABB,
    const std::function<bool(const AABB &)> &callback) const
{
    auto tilePositions = worldToTilePositions(probeAABB.position, probeAABB.size);
    for (const auto &tilePosition : tilePositions)
    {
        if (!validTilePosition(tilePosition))
            continue;

        const Tile &tile = getTileAtTilePosition(tilePosition);
        if (!tile.isSolid())
            continue;

        auto tileWorldPosition = tileToWorldPosition(tilePosition);
        AABB tileAABB = tile.getAABBAt(tileWorldPosition);
        if (tileAABB.intersects(probeAABB) && callback(tileAABB))
            return true;
    }
    return false;
}

const std::vector<NpcSpawnData> &TileMap::getNpcs() const
{
    return npcs;
}
