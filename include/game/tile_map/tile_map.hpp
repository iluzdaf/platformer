#pragma once
#include <vector>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "game/tile_map/tile.hpp"
#include "game/tile_map/tile_map_data.hpp"

class TileMap
{
public:
    explicit TileMap(const std::string &jsonFilePath);
    explicit TileMap(const TileMapData &tileMapData);
    void setTileIndex(int x, int y, int tile);
    int getTileIndex(int x, int y) const;
    int getTileIndex(glm::vec2 worldPosition) const;
    const Tile &getTile(int tileIndex) const;
    const Tile &getTile(int x, int y) const;
    const Tile &getTile(glm::vec2 worldPosition) const;
    int getWidth() const;
    int getHeight() const;
    int getWorldWidth() const;
    int getWorldHeight() const;
    int getTileSize() const;
    void update(float deltaTime);

private:
    int width = 0, height = 0, tileSize = 0;
    std::vector<std::vector<int>> tileIndices;
    std::unordered_map<int, Tile> tiles;
    void initByData(const TileMapData &tileMapData);
};