#pragma once

#include <utility>
#include <map>
#include <glaze/glaze.hpp>
#include <string>
#include <stdexcept>
#include "tile_map/tile_map.hpp"
#include "game/level_data.hpp"
#include "tile_map/tile_map_data.hpp"
#include "game/game_data.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "tile_map/tile_data.hpp"
#include "npc/npc_data.hpp"
#include <vector>
#include <memory>
#include "npc/npc_spawn_data.hpp"
#include "npc/npc.hpp"
#include "game/level.hpp"
#include "pickups/pickup_data.hpp"
#include "assets/asset_paths.hpp"

inline TilePaletteData paletteOf(std::map<int, TileData> tiles)
{
    TilePaletteData palette;
    palette.tileSet.texture = std::string(assets::TileSetTexture);
    palette.tiles = std::move(tiles);
    return palette;
}

inline const TilePaletteData &getDefaultTileDataMap()
{
    static const TilePaletteData map = []
    {
        TileData solid;
        solid.solid = solid.grippable = true;
        return paletteOf({{0, TileData{}}, {1, solid}});
    }();
    return map;
}

inline const TilePalettes &shippedPalettes()
{
    static const TilePalettes palettes = [] { return loadGameData().tilePalettes; }();
    return palettes;
}

inline const std::map<std::string, NpcData> &shippedNpcData()
{
    static const std::map<std::string, NpcData> npcData = [] { return loadGameData().npcData; }();
    return npcData;
}

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

inline std::vector<NpcSpawnData> spawnsIn(const Level &level)
{
    std::vector<NpcSpawnData> spawns;
    for (const std::unique_ptr<Npc> &npc : level.getNpcs())
        spawns.push_back(npc->getSpawn());

    return spawns;
}

inline const std::map<std::string, PickupData> &shippedPickupData()
{
    static const std::map<std::string, PickupData> pickupData = []
    { return loadGameData().pickupData; }();
    return pickupData;
}

inline int aTileIn(const TilePaletteData &palette, bool TileData::*what)
{
    for (const auto &[tileIndex, tileData] : palette.tiles)
        if (tileData.*what)
            return tileIndex;

    throw std::runtime_error("no tile in this palette is what the test needs");
}

inline int aSolidTileIn(const TilePaletteData &palette)
{
    return aTileIn(palette, &TileData::solid);
}

inline int aDeadlyTileIn(const TilePaletteData &palette)
{
    return aTileIn(palette, &TileData::deadly);
}

inline TilePalettes palettesFrom(const TilePaletteData &palette)
{
    return {{"default", palette}};
}

inline TileMap setupTileMap(
    int width = 10,
    int height = 10,
    int tileSize = 16,
    const TilePaletteData &palette = getDefaultTileDataMap())
{
    TilePaletteData sized = palette;
    sized.tileSet.cellSize = glm::ivec2(tileSize);

    TileMapData tileMapData;
    tileMapData.tilePalette = "default";
    tileMapData.indices = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
    return TileMap(tileMapData, palettesFrom(sized));
}

inline TileMap setupTileMapWith(
    const std::vector<std::pair<glm::ivec2, int>> &placed,
    int width = 10,
    int height = 10,
    int tileSize = 16,
    const TilePaletteData &palette = getDefaultTileDataMap())
{
    TilePaletteData sized = palette;
    sized.tileSet.cellSize = glm::ivec2(tileSize);

    TileMapData tileMapData;
    tileMapData.tilePalette = "default";
    tileMapData.indices = std::vector<std::vector<int>>(height, std::vector<int>(width, 0));
    for (const auto &[tile, tileIndex] : placed)
        tileMapData.indices[tile.y][tile.x] = tileIndex;

    return TileMap(tileMapData, palettesFrom(sized));
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
