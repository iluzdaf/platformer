#pragma once

#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "helpers/palettes.hpp"
#include "helpers/tile_positions.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_map_data.hpp"
#include "tile_map/tile_palette_data.hpp"

using Placed = std::vector<std::pair<glm::ivec2, int>>;

inline void layRow(Placed &laid, int row, int fromX, int toX, int tile = SolidTile)
{
    for (int x = fromX; x <= toX; ++x)
        laid.push_back({glm::ivec2(x, row), tile});
}

inline void layColumn(Placed &laid, int column, int fromY, int toY, int tile = SolidTile)
{
    for (int y = fromY; y <= toY; ++y)
        laid.push_back({glm::ivec2(column, y), tile});
}

inline TileMapData aTileMapData(const Placed &placed = {}, int width = 10, int height = 10)
{
    TileMapData tileMapData;
    tileMapData.tilePalette = "default";
    tileMapData.indices = std::vector<std::vector<int>>(height, std::vector<int>(width, EmptyTile));
    for (const auto &[tile, tileIndex] : placed)
        tileMapData.indices[tile.y][tile.x] = tileIndex;

    return tileMapData;
}

inline TileMap aTileMap(
    const Placed &placed = {},
    int width = 10,
    int height = 10,
    int tileSize = TestTileSize,
    const TilePaletteData &palette = aPaletteWithASolidTile())
{
    TilePaletteData sized = palette;
    sized.tileSet.cellSize = glm::ivec2(tileSize);

    return TileMap(aTileMapData(placed, width, height), theOnlyPalette(sized));
}
