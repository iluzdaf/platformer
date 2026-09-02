#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "tile_map/tile.hpp"
#include "tile_map/tile_map_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "assets/sheet.hpp"
#include "physics/aabb.hpp"

class TileMap
{
public:
    TileMap(const TileMapData &tileMapData, const TilePalettes &tilePalettes);
    void setTileIndex(glm::ivec2 tilePosition, int tileIndex);
    void setTileIndexAt(glm::vec2 worldPosition, int tileIndex);
    int tilePositionToTileIndex(glm::ivec2 tilePosition) const;
    int tileIndexAt(glm::vec2 worldPosition) const;
    glm::ivec2 tileContaining(glm::vec2 worldPosition) const;
    const Tile &getTile(int tileIndex) const;
    const Tile &getTileAtTilePosition(glm::ivec2 tilePosition) const;
    const Tile &getTileAtWorldPosition(glm::vec2 worldPosition) const;
    int getWidth() const;
    int getHeight() const;
    int getWorldWidth() const;
    int getWorldHeight() const;
    int getTileSize() const;
    void update(float deltaTime);
    std::vector<glm::ivec2> tilesOverlapping(glm::vec2 worldPosition, glm::vec2 size) const;
    glm::vec2 topLeftOfTile(glm::ivec2 tilePosition) const;
    glm::vec2 feetOnTile(glm::ivec2 tilePosition) const;
    glm::ivec2 tileStoodOnAt(glm::vec2 worldPosition) const;
    bool standsOnGround(glm::ivec2 tilePosition) const;
    TileMapData toTileMapData() const;
    bool validTilePosition(glm::ivec2 tilePosition) const;
    bool probeSolidTiles(
        const AABB &probeAABB,
        const std::function<bool(const Tile &, const AABB &)> &callback) const;

private:
    int width = 0, height = 0, tileSize = 0;
    std::vector<std::vector<int>> tileIndices;
    std::unordered_map<int, Tile> tiles;
    std::string tilePalette;
    Sheet tileSet;

public:
    const std::string &getTilePalette() const;
    void setTilePalette(const std::string &name);
    const Sheet &getTileSet() const;

private:
    void initFrom(const TileMapData &tileMapData, const TilePalettes &tilePalettes);
};