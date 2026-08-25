#pragma once

#include <glaze/glaze.hpp>
#include "game/tile_map/tile_map.hpp"
#include "game/tile_map/tile_map_data.hpp"
#include "game/game_data.hpp"

inline const TilePalette &getDefaultTileDataMap()
{
    static const TilePalette map = {
        {0, TileData{TileKind::Empty, std::nullopt, std::nullopt, std::nullopt, glm::vec2(0.0f), glm::vec2(16.0f)}},
        {1, TileData{TileKind::Solid, std::nullopt, std::nullopt, std::nullopt, glm::vec2(0.0f), glm::vec2(16.0f)}}};
    return map;
}

inline const TilePalettes &shippedPalettes()
{
    static const TilePalettes palettes = []
    {
        GameData gameData;
        auto ec = glz::read_file_json(gameData, "../../assets/game_data.json", std::string{});
        if (ec)
            throw std::runtime_error("Failed to read game_data.json");

        return gameData.tilePalettes;
    }();
    return palettes;
}

inline TilePalettes palettesFrom(const TilePalette &palette)
{
    return {{"default", palette}};
}

inline TileMap setupTileMap(
    int width = 10,
    int height = 10,
    int tileSize = 16,
    const TilePalette &palette = getDefaultTileDataMap())
{
    TileMapData tileMapData;
    tileMapData.size = tileSize;
    tileMapData.width = width;
    tileMapData.height = height;
    return TileMap(tileMapData, palettesFrom(palette));
}