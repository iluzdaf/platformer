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

void TileMap::setTiles(const std::unordered_map<int, TileDefinition> &tileDefinitions)
{
    tiles.clear();

    for (const auto &[index, definition] : tileDefinitions)
    {
        Tile tile(definition.kind);

        if (definition.animation.has_value())
        {
            tile.setAnimation(definition.animation.value());
        }

        tiles[index] = std::move(tile);
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

void TileMap::update(float deltaTime)
{
    for (auto &[_, tile] : tiles)
    {
        tile.update(deltaTime);
    }
}