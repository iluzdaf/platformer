#include <cassert>
#include <glaze/glaze.hpp>
#include "game/tile_map/tile_map.hpp"

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

        for (int y = 0; y < height; ++y)
        {
            if (indices[y].size() != width)
            {
                throw std::runtime_error("Inconsistent row width in tileIndices");
            }
        }

        tileIndices = std::vector<std::vector<int>>(width, std::vector<int>(height, -1));
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                tileIndices[x][y] = indices[y][x];
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
}

void TileMap::setTileIndex(int x, int y, int tile)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("Coordinates out of bounds");
    }

    if (tile < 0)
    {
        throw std::invalid_argument("Tile index must be greater or equals to 0");
    }

    tileIndices[x][y] = tile;
}

int TileMap::getTileIndex(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        return -1;
    }

    return tileIndices[x][y];
}

int TileMap::getTileIndex(glm::vec2 worldPosition) const
{
    int x = static_cast<int>(worldPosition.x) / tileSize;
    int y = static_cast<int>(worldPosition.y) / tileSize;
    return getTileIndex(x, y);
}

int TileMap::getWidth() const { return width; }

int TileMap::getHeight() const { return height; }

const Tile &TileMap::getTile(int tileIndex) const
{
    auto it = tiles.find(tileIndex);
    if (it == tiles.end())
    {
        return tiles.at(-1);
    }
    return it->second;
}

const Tile &TileMap::getTile(int x, int y) const
{
    int tileIndex = getTileIndex(x, y);
    return getTile(tileIndex);
}

const Tile &TileMap::getTile(glm::vec2 worldPosition) const
{
    int tileIndex = getTileIndex(worldPosition);
    return getTile(tileIndex);
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