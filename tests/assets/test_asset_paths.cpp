#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include "assets/asset_paths.hpp"

TEST_CASE("The assets root is somewhere that exists", "[AssetPaths]")
{
    REQUIRE(std::filesystem::is_directory(assets::root()));
    REQUIRE(std::filesystem::path(assets::root()).is_absolute());
}

TEST_CASE("A named asset resolves to a file that is there", "[AssetPaths]")
{
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::GameData)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::LevelList)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::FirstLevel)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::GameLogicScript)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::TileSetTexture)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::PlayerTexture)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::TileSetVertexShader)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::TileSetFragmentShader)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::TransitionVertexShader)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::TransitionFragmentShader)));
}

TEST_CASE("Naming an asset and reading a path back agree", "[AssetPaths]")
{
    REQUIRE(assets::underRoot(assets::pathTo(assets::FirstLevel)) == "levels/level1.json");
    REQUIRE(assets::underRoot(assets::pathTo(assets::TileSetTexture)) == "textures/tile_set.png");
}

TEST_CASE("A path the file watcher reports is named the same way", "[AssetPaths]")
{
    std::filesystem::path reported =
        std::filesystem::path(assets::root()) / "levels" / "level6.json";

    REQUIRE(assets::underRoot(reported.string()) == "levels/level6.json");
}
