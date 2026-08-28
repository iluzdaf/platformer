#pragma once

#include <string>
#include <string_view>

// Everything the game loads is named here once, relative to the assets root,
// and resolved to a filesystem path only where a file is actually opened.
// A level saying "levels/level2.json" then means the same thing to the loader,
// the editor and the file watcher, none of which need to know where the root is.
namespace assets
{
    inline constexpr std::string_view Levels = "levels";
    inline constexpr std::string_view Scripts = "scripts";
    inline constexpr std::string_view Shaders = "shaders";
    inline constexpr std::string_view Textures = "textures";

    inline constexpr std::string_view GameData = "game_data.json";
    inline constexpr std::string_view LevelList = "levels.json";
    inline constexpr std::string_view FirstLevel = "levels/level1.json";
    inline constexpr std::string_view GameLogicScript = "scripts/game_logic.lua";

    inline constexpr std::string_view TileSetTexture = "textures/tile_set.png";
    inline constexpr std::string_view PlayerTexture = "textures/player.png";

    inline constexpr std::string_view TileSetVertexShader = "shaders/tile_set.vs";
    inline constexpr std::string_view TileSetFragmentShader = "shaders/tile_set.fs";
    inline constexpr std::string_view TransitionVertexShader = "shaders/transition.vs";
    inline constexpr std::string_view TransitionFragmentShader = "shaders/transition.fs";

    const std::string &root();
    std::string pathTo(std::string_view relative);
    std::string underRoot(const std::string &path);
}
