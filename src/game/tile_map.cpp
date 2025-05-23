#include "game/tile_map.hpp"
#include <cassert>

TileMap::TileMap(int width, int height, int tileSize) : width(width),
                                                        height(height),
                                                        tiles(width, std::vector<int>(height, -1)),
                                                        tileSize(tileSize)
{
    assert(width > 0);
    assert(height > 0);
    assert(tileSize > 0);
}

void TileMap::setTile(int x, int y, int tile)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("Coordinates out of bounds");
    }

    if (tile < 0)
    {
        throw std::invalid_argument("Tile index must be greater or equals to 0");
    }

    tiles[x][y] = tile;
}

int TileMap::getTile(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("Coordinates out of bounds");
    }

    return tiles[x][y];
}

int TileMap::getWidth() const { return width; }

int TileMap::getHeight() const { return height; }

void TileMap::setTileTypes(const std::unordered_map<int, TileKind> &types)
{
    tileTypes.clear();
    for (auto &[index, kind] : types)
    {
        tileTypes[index] = TileType{kind};
    }
}

const TileType &TileMap::getTileType(int tileIndex) const
{
    auto it = tileTypes.find(tileIndex);
    if (it != tileTypes.end())
    {
        return it->second;
    }

    static const TileType defaultType{TileKind::Empty};
    return defaultType;
}

int TileMap::getTileSize() const
{
    return tileSize;
}