#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <vector>
#include <glaze/glaze.hpp>
#include "game/levels.hpp"
#include "game/levels_data.hpp"
#include "game/game_data.hpp"
#include "ui/saveable.hpp"
#include "helpers/asset_path.hpp"
#include "helpers/levels.hpp"
#include "helpers/temporary_levels.hpp"
#include "helpers/tiles.hpp"
#include "game/empty_level.hpp"
#include "game/level_data.hpp"
#include "game/level_data_file.hpp"
#include "game/level_path_data.hpp"
#include <glm/gtc/matrix_transform.hpp>

TEST_CASE("The game data names the level the game starts on", "[Levels]")
{
    REQUIRE_FALSE(loadGameData().levels.first.path.empty());
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

TEST_CASE("A level path nobody has taken is free of the folder and of what is held", "[Levels]")
{
    TemporaryLevels levels{"free_level_paths"};
    std::string directory = levels.directory.string();
    levels.write("level1.json", aFloorLevelPlacing({}));

    std::string free = aLevelPathNobodyHasTaken(directory);
    REQUIRE(free != levels.pathOf("level1.json"));
    REQUIRE_FALSE(readLevelDataIfYouCan(free));

    REQUIRE(aLevelPathNobodyHasTaken(directory, {free}) != free);
}

TEST_CASE("An empty level is the size of the one it was made from", "[Levels]")
{
    LevelData playing = aFloorLevelPlacing({aVillagerAt(glm::ivec2(2, FloorLevelStanding))});
    playing.tileMapData.tilePalette = "cavern";
    playing.nextLevel = "levels/level3.json";

    LevelData made = anEmptyLevelLike(playing, static_cast<int>(TestTileSize));

    REQUIRE(made.tileMapData.indices.size() == playing.tileMapData.indices.size());
    REQUIRE(made.tileMapData.indices.front().size() == playing.tileMapData.indices.front().size());
    REQUIRE(made.tileMapData.tilePalette == "cavern");
    REQUIRE(made.nextLevel == LevelPathData("levels/level3.json"));
    REQUIRE(made.npcs.empty());
    REQUIRE(made.pickups.empty());

    for (const std::vector<int> &row : made.tileMapData.indices)
        for (int tile : row)
            REQUIRE(tile == 0);
}

TEST_CASE("An empty level stands the player on its bottom row", "[Levels]")
{
    LevelData playing = aFloorLevelPlacing({});

    LevelData made = anEmptyLevelLike(playing, static_cast<int>(TestTileSize));

    REQUIRE(made.playerFeet == feetOf(glm::ivec2(0, FloorLevelTiles - 1)));
}
