#include <glaze/glaze.hpp>
#include "game/tile_map/tile_map.hpp"

TileMap::TileMap(const std::string &jsonFilePath) : level(jsonFilePath)
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

    initByData(tileMapData);
}

TileMap::TileMap(const TileMapData &tileMapData)
{
    initByData(tileMapData);
}

void TileMap::initByData(const TileMapData &tileMapData)
{
    tileSize = tileMapData.size;

    if (tileSize <= 0)
        throw std::runtime_error("tileSize must be greater than 0");

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
    {
        throw std::runtime_error("Must specify either tileIndices or width/height.");
    }

    if (width == 0 || height == 0)
    {
        throw std::runtime_error("TileMapData has invalid dimensions");
    }

    tiles.insert_or_assign(0, Tile(TileData{TileKind::Empty}));
    for (const auto &[tileIndex, tileData] : tileMapData.tileData)
    {
        tiles.insert_or_assign(tileIndex, Tile(tileData));
    }

    playerStartTilePosition = tileMapData.playerStartTilePosition;
    if (playerStartTilePosition.x < 0 || playerStartTilePosition.y < 0 ||
        playerStartTilePosition.x >= width || playerStartTilePosition.y >= height)
        throw std::runtime_error("playerStartTilePosition is out of bounds");

    const Tile &tile = getTileAtTilePosition(playerStartTilePosition);
    if (tile.isSolid())
        throw std::runtime_error("Player start position is on a solid tile");
    if (tile.isSpikes())
        throw std::runtime_error("Player start position is on a spike tile");

    nextLevel = tileMapData.nextLevel;
    if (nextLevel.empty())
        throw std::runtime_error("nextLevel must not be empty");
}

void TileMap::setTileIndex(glm::ivec2 tilePosition, int tileIndex)
{
    if (!validTilePosition(tilePosition))
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
    {
        throw std::out_of_range("Tile coordinates out of bounds");
    }

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
    {
        throw std::out_of_range("Invalid tile index");
    }
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
    int tileMinX = floor(worldPosition.x / tileSize);
    int tileMaxX = floor(worldPositionMax.x / tileSize);
    int tileMinY = floor(worldPosition.y / tileSize);
    int tileMaxY = floor(worldPositionMax.y / tileSize);
    for (int tileY = tileMinY; tileY <= tileMaxY; ++tileY)
    {
        for (int tileX = tileMinX; tileX <= tileMaxX; ++tileX)
        {
            glm::ivec2 tilePosition(tileX, tileY);
            if (!validTilePosition(tilePosition))
            {
                continue;
            }
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
    for (const auto &[index, tile] : tiles)
        data.tileData[index] = tile.toTileData();
    return data;
}

void TileMap::save() const
{
    TileMapData data = toTileMapData();
    std::string json;
    auto result = glz::write_json(data, json);
    if (result)
    {
        throw std::runtime_error("Failed to serialize TileMapData to JSON");
    }
    std::ofstream outFile(level);
    outFile << json;
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
        if (tileAABB.intersects(probeAABB))
        {
            if (callback(tileAABB))
                return true;
        }
    }
    return false;
}