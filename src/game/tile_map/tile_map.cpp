#include <cassert>
#include <glaze/glaze.hpp>
#include "game/tile_map/tile_map.hpp"
#include "game/tile_map/tile_map_data.hpp"

TileMap::TileMap(const std::string &jsonFilePath)
{
    if (jsonFilePath.empty())
    {
        throw std::runtime_error("jsonFilePath is empty");
    }

    TileMapData tileMapData;
    auto ec = glz::read_file_json(tileMapData, jsonFilePath, std::string{});
    if (ec)
    {
        throw std::runtime_error("Failed to read json file");
    }

    initByData(tileMapData);
}

TileMap::TileMap(const TileMapData &tileMapData)
{
    initByData(tileMapData);
}

void TileMap::initByData(const TileMapData &tileMapData)
{
    tileSize = tileMapData.size;

    assert(tileSize > 0);

    const bool hasTileIndices = tileMapData.indices.has_value();
    const bool hasExplicitSize = tileMapData.width.has_value() && tileMapData.height.has_value();

    if (hasTileIndices && hasExplicitSize)
    {
        throw std::runtime_error("Cannot specify both tileIndices and width/height explicitly.");
    }

    if (hasTileIndices)
    {
        const auto &indices = tileMapData.indices.value();
        height = indices.size();
        width = height > 0 ? indices[0].size() : 0;

        for (int tileY = 0; tileY < height; ++tileY)
        {
            if (indices[tileY].size() != width)
            {
                throw std::runtime_error("Inconsistent row width in tileIndices");
            }
        }

        tileIndices = std::vector<std::vector<int>>(width, std::vector<int>(height, -1));
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
        tileIndices = std::vector<std::vector<int>>(width, std::vector<int>(height, -1));
    }
    else
    {
        throw std::runtime_error("Must specify either tileIndices or width/height.");
    }

    if (width == 0 || height == 0)
    {
        throw std::runtime_error("TileMapData has invalid dimensions");
    }

    tiles.insert_or_assign(-1, Tile(TileData{TileKind::Empty}));
    for (const auto &[tileIndex, tileData] : tileMapData.tileData)
    {
        tiles.insert_or_assign(tileIndex, Tile(tileData));
    }

    playerStartTilePosition = tileMapData.playerStartTilePosition;
    assert(playerStartTilePosition.x >= 0);
    assert(playerStartTilePosition.y >= 0);
    assert(playerStartTilePosition.x < width);
    assert(playerStartTilePosition.y < height);
    const Tile &tile = getTile(playerStartTilePosition);
    assert(!tile.isSolid());
    assert(!tile.isSpikes());

    nextLevel = tileMapData.nextLevel;
    assert(!nextLevel.empty());
}

void TileMap::setTileIndex(glm::ivec2 tilePosition, int tileIndex)
{
    if (tilePosition.x < 0 ||
        tilePosition.x >= width ||
        tilePosition.y < 0 ||
        tilePosition.y >= height)
    {
        throw std::out_of_range("Tile coordinates out of bounds");
    }

    if (tileIndex < 0)
    {
        throw std::invalid_argument("Tile index must be greater or equals to 0");
    }

    tileIndices[tilePosition.x][tilePosition.y] = tileIndex;
}

void TileMap::setTileIndexAt(glm::vec2 worldPosition, int tileIndex)
{
    setTileIndex(getTileCoordinates(worldPosition), tileIndex);
}

int TileMap::getTileIndex(glm::ivec2 tilePosition) const
{
    if (tilePosition.x < 0 ||
        tilePosition.x >= width ||
        tilePosition.y < 0 ||
        tilePosition.y >= height)
    {
        return -1;
    }

    return tileIndices[tilePosition.x][tilePosition.y];
}

glm::ivec2 TileMap::getTileCoordinates(glm::vec2 worldPosition) const
{
    return glm::ivec2(static_cast<int>(worldPosition.x) / tileSize, static_cast<int>(worldPosition.y) / tileSize);
}

int TileMap::getTileIndexAt(glm::vec2 worldPosition) const
{
    return getTileIndex(getTileCoordinates(worldPosition));
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
    {
        return tiles.at(-1);
    }
    return it->second;
}

const Tile &TileMap::getTile(glm::ivec2 tilePosition) const
{
    return getTile(getTileIndex(tilePosition));
}

const Tile &TileMap::getTileAt(glm::vec2 worldPosition) const
{
    return getTile(getTileIndexAt(worldPosition));
}

int TileMap::getTileSize() const
{
    return tileSize;
}

void TileMap::update(float deltaTime)
{
    for (auto &[_, tile] : tiles)
    {
        tile.update(deltaTime);
    }
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
    return glm::vec2(playerStartTilePosition.x * tileSize, playerStartTilePosition.y * tileSize);
}

const std::string &TileMap::getNextLevel() const
{
    return nextLevel;
}