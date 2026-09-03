#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <vector>
#include <glaze/glaze.hpp>
#include "game/levels.hpp"
#include "game/levels_data.hpp"
#include "game/game_data.hpp"
#include "ui/saveable.hpp"
#include "test_helpers/asset_path.hpp"

TEST_CASE("The game data names the level the game starts on", "[Levels]")
{
    REQUIRE_FALSE(loadGameData().levels.first.empty());
}

TEST_CASE("The level the game starts on survives being written and read", "[Levels]")
{
    LevelsData levels{"levels/level4.json"};

    LevelsData back;
    REQUIRE_FALSE(glz::read_json(back, asJson(levels)));
    REQUIRE(back.first == "levels/level4.json");
}

TEST_CASE("The levels folder lists every level it holds", "[Levels]")
{
    std::vector<std::string> paths = levelPathsIn(assetPath("levels"));

    REQUIRE(paths.size() >= 6);
    REQUIRE(std::is_sorted(paths.begin(), paths.end()));
    for (const std::string &path : paths)
        REQUIRE(path.ends_with(".json"));
}
