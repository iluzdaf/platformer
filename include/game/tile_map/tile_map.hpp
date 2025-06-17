#pragma once
#include <vector>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "game/tile_map/tile.hpp"
#include "game/tile_map/tile_map_data.hpp"
#include "physics/aabb.hpp"

class TileMap
{
public:
    explicit TileMap(const std::string &jsonFilePath);
    explicit TileMap(const TileMapData &tileMapData);
    void setTileIndex(glm::ivec2 tilePosition, int tileIndex);
    void setTileIndexAt(glm::vec2 worldPosition, int tileIndex);
    int tilePositionToTileIndex(glm::ivec2 tilePosition) const;
    int worldPositionToTileIndex(glm::vec2 worldPosition) const;
    glm::ivec2 worldToTilePosition(glm::vec2 worldPosition) const;
    const Tile &getTile(int tileIndex) const;
    const Tile &getTileAtTilePosition(glm::ivec2 tilePosition) const;
    const Tile &getTileAtWorldPosition(glm::vec2 worldPosition) const;
    int getWidth() const;
    int getHeight() const;
    int getWorldWidth() const;
    int getWorldHeight() const;
    int getTileSize() const;
    void update(float deltaTime);
    glm::vec2 getPlayerStartWorldPosition() const;
    const std::string &getNextLevel() const;
    std::vector<glm::ivec2> worldToTilePositions(glm::vec2 worldPosition, glm::vec2 size) const;
    glm::vec2 tileToWorldPosition(glm::ivec2 tilePosition) const;
    const std::unordered_map<int, Tile> &getTiles() const;
    TileMapData toTileMapData() const;
    void save() const;
    void setPlayerStartTile(glm::ivec2 tilePosition);
    bool validTilePosition(glm::ivec2 tilePosition) const;
    const std::string &getLevel() const;
    bool probeSolidTiles(
        const AABB &probeAABB,
        const std::function<bool(const AABB &)> &callback) const;

private:
    int width = 0, height = 0, tileSize = 0;
    std::vector<std::vector<int>> tileIndices;
    std::unordered_map<int, Tile> tiles;
    void initByData(const TileMapData &tileMapData);
    glm::ivec2 playerStartTilePosition = glm::ivec2(0, 0);
    std::string nextLevel = "../assets/levels/level1.json";
    std::string level = "../assets/levels/new_level.json";
};