#include "game/tile_map.hpp"
#include <cassert>

TileMap::TileMap(int width, int height, int tileSize) : width(width),
                                                        height(height),
                                                        tileIndices(width, std::vector<int>(height, -1)),
                                                        tileSize(tileSize)
{
    assert(width > 0);
    assert(height > 0);
    assert(tileSize > 0);
}

void TileMap::setTileIndex(int x, int y, int tile)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("Coordinates out of bounds");
    }

    if (tile < 0)
    {
        throw std::invalid_argument("Tile index must be greater or equals to 0");
    }

    tileIndices[x][y] = tile;
}

int TileMap::getTileIndex(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        return -1;
    }

    return tileIndices[x][y];
}

int TileMap::getWidth() const { return width; }

int TileMap::getHeight() const { return height; }

void TileMap::setTiles(const std::unordered_map<int, TileKind> &tiles)
{
    this->tiles.clear();
    for (auto &[index, kind] : tiles)
    {
        this->tiles[index] = Tile(kind);
    }
}

std::optional<std::reference_wrapper<const Tile>> TileMap::getTile(int tileIndex) const
{
    auto it = tiles.find(tileIndex);
    if (it == tiles.end())
    {
        return std::nullopt;
    }
    return it->second;
}

int TileMap::getTileSize() const
{
    return tileSize;
}