#pragma once

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <glaze/glaze.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level_data.hpp"
#include "helpers/palettes.hpp"
#include "helpers/shipped.hpp"
#include "npc/npc_spawn_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_map_data.hpp"
#include "tile_map/tile_palette_data.hpp"

constexpr float TestTileSize = 16.0f;

inline glm::vec2 feetOf(glm::ivec2 tile, float tileSize = TestTileSize)
{
    return glm::vec2(tile.x * tileSize + tileSize * 0.5f, (tile.y + 1) * tileSize);
}

inline glm::vec2 middleOf(glm::ivec2 tile, float tileSize = TestTileSize)
{
    return glm::vec2(tile.x * tileSize + tileSize * 0.5f, tile.y * tileSize + tileSize * 0.5f);
}

inline PatrolData beatOf(glm::ivec2 fromTile, glm::ivec2 toTile, float tileSize = TestTileSize)
{
    glm::vec2 from = feetOf(fromTile, tileSize);
    glm::vec2 to = feetOf(toTile, tileSize);
    float outwards = tileSize * 0.5f;

    if (from.x <= to.x)
    {
        from.x -= outwards;
        to.x += outwards;
    }
    else
    {
        from.x += outwards;
        to.x -= outwards;
    }

    return PatrolData{from, to};
}

inline TileMap aTileMap(
    int width = 10,
    int height = 10,
    int tileSize = 16,
    const TilePaletteData &palette = aPaletteWithASolidTile())
{
    TilePaletteData sized = palette;
    sized.tileSet.cellSize = glm::ivec2(tileSize);

    TileMapData tileMapData;
    tileMapData.tilePalette = "default";
    tileMapData.indices = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
    return TileMap(tileMapData, theOnlyPalette(sized));
}

inline TileMap aTileMapWith(
    const std::vector<std::pair<glm::ivec2, int>> &placed,
    int width = 10,
    int height = 10,
    int tileSize = 16,
    const TilePaletteData &palette = aPaletteWithASolidTile())
{
    TilePaletteData sized = palette;
    sized.tileSet.cellSize = glm::ivec2(tileSize);

    TileMapData tileMapData;
    tileMapData.tilePalette = "default";
    tileMapData.indices = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
    for (const auto &[tile, tileIndex] : placed)
        tileMapData.indices[tile.y][tile.x] = tileIndex;

    return TileMap(tileMapData, theOnlyPalette(sized));
}

inline TileMap tilesOfLevel(const std::string &jsonFilePath)
{
    LevelData levelData;
    levelData.playerStart = feetOf(glm::ivec2(0, 0));
    auto error = glz::read_file_json(levelData, jsonFilePath, std::string{});
    if (error)
        throw std::runtime_error("Failed to read " + jsonFilePath);

    return TileMap(levelData.tileMapData, shippedPalettes());
}
