#pragma once
#include "game/tile.hpp"
#include "game/tile_map_data.hpp"
#include <vector>
#include <unordered_map>

class TileMap
{
public:
    explicit TileMap(const std::string& jsonFilePath);
    explicit TileMap(const TileMapData& tileMapData);
    void setTileIndex(int x, int y, int tile);
    int getTileIndex(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
    const Tile& getTile(int tileIndex) const;
    int getTileSize() const;
    void update(float deltaTime);
    int getWorldWidth() const;
    int getWorldHeight() const;

private:
    int width = 0, height = 0, tileSize = 0;
    std::vector<std::vector<int>> tileIndices;
    std::unordered_map<int, Tile> tiles;
    void initByData(const TileMapData &tileMapData);
};