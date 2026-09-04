#include <algorithm>
#include <optional>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <glaze/glaze.hpp>
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_map_data.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_data.hpp"
#include "physics/aabb.hpp"

TileMap::TileMap(const TileMapData &tileMapData, const TilePalettes &tilePalettes)
{
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
                if (indices[tileY][tileX] < 0)
                    throw std::runtime_error("Tile index must be greater or equals to 0");

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
    if (tilePalette.empty())
        throw std::runtime_error("This level names no tile palette");

    auto palette = tilePalettes.find(tilePalette);
    if (palette == tilePalettes.end())
        throw std::runtime_error("Unknown tile palette \"" + tilePalette + "\"");

    tileSet = palette->second.tileSet;
    if (tileSet.texture.empty())
        throw std::runtime_error("Palette \"" + tilePalette + "\" names no tile set texture");

    if (tileSet.cellSize.x <= 0)
        throw std::runtime_error("Palette \"" + tilePalette + "\" has a tile set cell size of 0");

    if (tileSet.cellSize.x != tileSet.cellSize.y)
        throw std::runtime_error(
            "Palette \"" + tilePalette + "\" has cells " + std::to_string(tileSet.cellSize.x) +
            " by " + std::to_string(tileSet.cellSize.y) + ", and a tile map lays out squares");

    tileSize = tileSet.cellSize.x;

    for (const auto &[tileIndex, tileData] : palette->second.tiles)
    {
        if (tileIndex < 0)
            throw std::runtime_error("Palette \"" + tilePalette + "\" names a tile below 0");

        tiles.insert_or_assign(tileIndex, Tile(tileData, glm::vec2(tileSet.cellSize)));
    }
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

glm::ivec2 TileMap::tileContaining(glm::vec2 worldPosition) const
{
    return glm::ivec2(
        static_cast<int>(worldPosition.x) / tileSize, static_cast<int>(worldPosition.y) / tileSize);
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
    static const Tile nothingSaid{TileData{}, glm::vec2(0.0f)};

    auto said = tiles.find(tileIndex);

    return said == tiles.end() ? nothingSaid : said->second;
}

const Tile &TileMap::getTileAtTilePosition(glm::ivec2 tilePosition) const
{
    return getTile(tilePositionToTileIndex(tilePosition));
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

std::vector<glm::ivec2> TileMap::tilesOverlapping(glm::vec2 worldPosition, glm::vec2 size) const
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

glm::vec2 TileMap::topLeftOfTile(glm::ivec2 tilePosition) const
{
    return glm::vec2(tilePosition.x * tileSize, tilePosition.y * tileSize);
}

glm::vec2 TileMap::feetOnTile(glm::ivec2 tilePosition) const
{
    if (!validTilePosition(tilePosition))
        throw std::runtime_error("Tile coordinates out of bounds");

    glm::vec2 surface = topLeftOfTile(tilePosition + glm::ivec2(0, 1));
    return surface + glm::vec2(tileSize * 0.5f, 0.0f);
}

glm::vec2 TileMap::middleOfTile(glm::ivec2 tilePosition) const
{
    return topLeftOfTile(tilePosition) + glm::vec2(tileSize * 0.5f);
}

glm::ivec2 TileMap::tileUnderFeet(glm::vec2 feet) const
{
    float tileSizePixels = static_cast<float>(tileSize);

    return glm::ivec2(
        static_cast<int>(std::floor(feet.x / tileSizePixels)),
        static_cast<int>(std::round(feet.y / tileSizePixels)) - 1);
}

glm::ivec2 TileMap::tileStoodOnAt(glm::vec2 worldPosition) const
{
    float tileSizePixels = static_cast<float>(tileSize);

    int column = static_cast<int>(std::floor(worldPosition.x / tileSizePixels));
    int row = static_cast<int>(std::round(worldPosition.y / tileSizePixels)) - 1;

    glm::ivec2 standingOn(std::clamp(column, 0, width - 1), std::clamp(row, 0, height - 1));
    if (standsOnGround(standingOn))
        return standingOn;

    glm::ivec2 leftOfIt(standingOn.x - 1, standingOn.y);

    return standsOnGround(leftOfIt) ? leftOfIt : standingOn;
}

bool TileMap::standsOnGround(glm::ivec2 tilePosition) const
{
    glm::ivec2 below(tilePosition.x, tilePosition.y + 1);

    return validTilePosition(tilePosition) && validTilePosition(below) &&
           !getTileAtTilePosition(tilePosition).isSolid() && getTileAtTilePosition(below).isSolid();
}

TileMapData TileMap::toTileMapData() const
{
    TileMapData data;
    data.indices = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            (*data.indices)[y][x] = tileIndices[x][y];
    data.tilePalette = tilePalette;
    return data;
}

bool TileMap::probeSolidTiles(
    const AABB &probeAABB,
    const std::function<bool(const Tile &, const AABB &)> &callback) const
{
    auto tilePositions = tilesOverlapping(probeAABB.position, probeAABB.size);
    for (const auto &tilePosition : tilePositions)
    {
        const Tile &tile = getTileAtTilePosition(tilePosition);
        if (!tile.isSolid())
            continue;

        auto tileWorldPosition = topLeftOfTile(tilePosition);
        std::optional<AABB> tileAABB = tile.getAABBAt(tileWorldPosition);
        if (tileAABB && tileAABB->intersects(probeAABB) && callback(tile, *tileAABB))
            return true;
    }
    return false;
}

const SheetData &TileMap::getTileSet() const
{
    return tileSet;
}
