#pragma once

#include <string>
#include <string_view>

namespace assets
{
    inline constexpr std::string_view Data = "data";
    inline constexpr std::string_view Levels = "levels";
    inline constexpr std::string_view Scripts = "scripts";
    inline constexpr std::string_view Shaders = "shaders";
    inline constexpr std::string_view Textures = "textures";

    inline constexpr std::string_view GameSettings = "data/game.json";
    inline constexpr std::string_view Camera = "data/camera.json";
    inline constexpr std::string_view Player = "data/player.json";
    inline constexpr std::string_view Npcs = "data/npcs.json";
    inline constexpr std::string_view Pickups = "data/pickups.json";
    inline constexpr std::string_view TilePalettes = "data/tile_palettes.json";
    inline constexpr std::string_view LevelList = "data/levels.json";
    inline constexpr std::string_view FirstLevel = "levels/level1.json";
    inline constexpr std::string_view GameLogicScript = "scripts/game_logic.lua";

    inline constexpr std::string_view TileSetTexture = "textures/cavern.png";
    inline constexpr std::string_view PlayerTexture = "textures/player.png";

    inline constexpr std::string_view TileSetVertexShader = "shaders/tile_set.vs";
    inline constexpr std::string_view TileSetFragmentShader = "shaders/tile_set.fs";
    inline constexpr std::string_view TransitionVertexShader = "shaders/transition.vs";
    inline constexpr std::string_view TransitionFragmentShader = "shaders/transition.fs";

    const std::string &root();
    std::string pathTo(std::string_view relative);
    std::string underRoot(const std::string &path);
}
