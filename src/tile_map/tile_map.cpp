#include <stdexcept>
#include <string>
#include <vector>
#include <glaze/glaze.hpp>
#include "tile_map/tile_map.hpp"

TileMap::TileMap(const TileMapData &tileMapData, const TilePalettes &tilePalettes)
{
    initFrom(tileMapData, tilePalettes);
}

void TileMap::initFrom(const TileMapData &tileMapData, const TilePalettes &tilePalettes)
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
    return (
        tilePosition.x >= 0 && tilePosition.x < width && tilePosition.y >= 0 &&
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
    return glm::ivec2(
        static_cast<int>(worldPosition.x) / tileSize, static_cast<int>(worldPosition.y) / tileSize);
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

glm::vec2 TileMap::tileToBottomCenterPosition(glm::ivec2 tilePosition) const
{
    glm::vec2 surface = tileToWorldPosition(tilePosition + glm::ivec2(0, 1));
    return surface + glm::vec2(tileSize * 0.5f, 0.0f);
}

const std::unordered_map<int, Tile> &TileMap::getTiles() const
{
    return tiles;
}

TileMapData TileMap::toTileMapData() const
{
    TileMapData data;
    data.size = tileSize;
    data.indices = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            (*data.indices)[y][x] = tileIndices[x][y];
    data.tilePalette = tilePalette;
    return data;
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
