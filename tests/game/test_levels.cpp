#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <filesystem>
#include "game/levels.hpp"
#include "test_helpers/asset_path.hpp"

TEST_CASE("Levels names the level the game starts on", "[Levels]")
{
    Levels levels(assetPath("levels.json"));

    REQUIRE_FALSE(levels.getFirst().empty());
}

TEST_CASE("Setting the first level and saving keeps it", "[Levels]")
{
    std::filesystem::path copy = std::filesystem::temp_directory_path() / "platformer_levels.json";
    std::filesystem::copy_file(
        assetPath("levels.json"), copy, std::filesystem::copy_options::overwrite_existing);

    Levels levels(copy.string());
    levels.setFirst("../../assets/levels/level4.json");
    levels.save();

    REQUIRE(Levels(copy.string()).getFirst() == "../../assets/levels/level4.json");
    std::filesystem::remove(copy);
}

TEST_CASE("Reading a levels file that is not there fails", "[Levels]")
{
    REQUIRE_THROWS(Levels("does_not_exist.json"));
}

TEST_CASE("The levels folder lists every level it holds", "[Levels]")
{
    std::vector<std::string> paths = levelPathsIn(assetPath("levels"));

    REQUIRE(paths.size() >= 6);
    REQUIRE(std::is_sorted(paths.begin(), paths.end()));
    for (const std::string &path : paths)
        REQUIRE(path.ends_with(".json"));
}
