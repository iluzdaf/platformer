#include <catch2/catch_test_macros.hpp>
#include <string_view>
#include "assets/asset_paths.hpp"
#include "reloading/game_data_watcher.hpp"

TEST_CASE("Every file loadGameData reads is one the watcher reloads on", "[Reloading]")
{
    for (std::string_view named :
         {assets::GameSettings,
          assets::Camera,
          assets::Player,
          assets::Npcs,
          assets::Pickups,
          assets::TilePalettes,
          assets::LevelList})
    {
        INFO(named);
        REQUIRE(GameDataWatcher::reloadsOn(named));
    }
}

TEST_CASE("A file that is not game data does not reload everything", "[Reloading]")
{
    REQUIRE_FALSE(GameDataWatcher::reloadsOn(assets::FirstLevel));
    REQUIRE_FALSE(GameDataWatcher::reloadsOn(assets::GameLogicScript));
    REQUIRE_FALSE(GameDataWatcher::reloadsOn(assets::TileSetTexture));
    REQUIRE_FALSE(GameDataWatcher::reloadsOn(assets::TileSetFragmentShader));
}
