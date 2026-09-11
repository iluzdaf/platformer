#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <algorithm>
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

    REQUIRE_FALSE(pngs.empty());
    REQUIRE(std::is_sorted(pngs.begin(), pngs.end()));
    for (const std::string &png : pngs)
    {
        REQUIRE(png.starts_with(std::string(assets::Textures) + "/"));
        REQUIRE(png.ends_with(".png"));
    }

    REQUIRE(assets::filesIn(assets::Textures, ".json").empty());
}

TEST_CASE("Files in a folder under the directory are listed too", "[AssetPaths]")
{
    std::vector<std::string> scripts = assets::filesIn(assets::Scripts, ".lua");

    REQUIRE(std::find(scripts.begin(), scripts.end(), "scripts/player.lua") != scripts.end());
    REQUIRE(std::find(scripts.begin(), scripts.end(), "scripts/npcs/rat.lua") != scripts.end());
}

TEST_CASE("A path outside the assets root is still said, and still found", "[AssetPaths]")
{
    std::string outside =
        (std::filesystem::temp_directory_path() / "platformer_outside.json").generic_string();

    std::string said = assets::underRoot(outside);

    REQUIRE_FALSE(said.empty());
    REQUIRE(
        std::filesystem::weakly_canonical(assets::pathTo(said)) ==
        std::filesystem::weakly_canonical(outside));
}
