#include "tile_map.hpp"
#include <cassert>

TileMap::TileMap(int width, int height) : width(width), height(height), tiles(width, std::vector<int>(height, 0))
{
    assert(width > 0);
    assert(height > 0);
}

void TileMap::setTile(int x, int y, int tile)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("Coordinates out of bounds");
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