#pragma once
#include <vector>
#include "game/tile_type.hpp"
#include <unordered_map>

class TileMap
{
public:
    TileMap(const int width, const int height, int tileSize = 16);
    void setTile(int x, int y, int tile);
    int getTile(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
    void setTileTypes(const std::unordered_map<int, TileKind> &types);
    const TileType& getTileType(int tileIndex) const;
    int getTileSize() const;

private:
    int width, height, tileSize;
    std::vector<std::vector<int>> tiles;
    std::unordered_map<int, TileType> tileTypes;
};