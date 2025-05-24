#pragma once
#include "game/tile.hpp"
#include "game/tile_definition.hpp"
#include <vector>
#include <unordered_map>
#include <optional>

class TileMap
{
public:
    TileMap(const int width, const int height, int tileSize = 16);
    void setTileIndex(int x, int y, int tile);
    int getTileIndex(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
    void setTiles(const std::unordered_map<int, TileDefinition> &tileDefinitions);
    std::optional<std::reference_wrapper<const Tile>> getTile(int tileIndex) const;
    int getTileSize() const;
    void update(float deltaTime);

private:
    int width, height, tileSize;
    std::vector<std::vector<int>> tileIndices;
    std::unordered_map<int, Tile> tiles;
};