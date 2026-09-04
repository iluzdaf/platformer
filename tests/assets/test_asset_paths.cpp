#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include "assets/asset_paths.hpp"

TEST_CASE("The assets root is somewhere that exists", "[AssetPaths]")
{
    REQUIRE(std::filesystem::is_directory(assets::root()));
    REQUIRE(std::filesystem::path(assets::root()).is_absolute());
}

TEST_CASE("A named asset resolves to a file that is there", "[AssetPaths]")
{
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::GameSettings)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::Camera)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::Player)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::Npcs)));
    REQUIRE(std::filesystem::exists(assets::pathTo(assets::TilePalettes)));
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
    REQUIRE(assets::underRoot(assets::pathTo(assets::FirstLevel)) == assets::FirstLevel);
    REQUIRE(assets::underRoot(assets::pathTo(assets::TileSetTexture)) == assets::TileSetTexture);
}

TEST_CASE("A path the file watcher reports is named the same way", "[AssetPaths]")
{
    std::filesystem::path reported =
        std::filesystem::path(assets::root()) / "levels" / "level6.json";

    REQUIRE(assets::underRoot(reported.string()) == "levels/level6.json");
}

TEST_CASE("Files under a directory are listed by extension in order", "[AssetPaths]")
{
    std::vector<std::string> pngs = assets::filesIn(assets::Textures, ".png");

    REQUIRE(
        pngs == std::vector<std::string>{
                    "textures/cavern.png",
                    "textures/coin.png",
                    "textures/explorer.png",
                    "textures/player.png",
                    "textures/villager.png"});
    REQUIRE(assets::filesIn(assets::Textures, ".json").empty());
}
