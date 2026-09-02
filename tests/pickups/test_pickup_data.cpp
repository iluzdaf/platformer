#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glaze/glaze.hpp>
#include "assets/sheet.hpp"
#include "animations/frame_animation_data.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "pickups/pickup_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "player/player_data.hpp"
#include "test_helpers/test_tile_map_utils.hpp"

namespace
{
    LevelData levelPlacing(const std::vector<PickupSpawnData> &pickups)
    {
        LevelData levelData;
        levelData.tileMapData.width = 4;
        levelData.tileMapData.height = 4;
        levelData.pickups = pickups;
        return levelData;
    }

    Level levelFrom(const LevelData &levelData)
    {
        return Level(
            levelData, palettesFrom(getDefaultTileDataMap()), PlayerData(), shippedNpcData());
    }
}

TEST_CASE("A level remembers the pickups it places", "[Pickups]")
{
    Level level = levelFrom(levelPlacing({{"coin", glm::ivec2(2, 3)}, {"gem", glm::ivec2(1, 1)}}));

    REQUIRE(level.getPickups().size() == 2);
    REQUIRE(level.getPickups()[0].type == "coin");
    REQUIRE(level.getPickups()[0].tilePosition == glm::ivec2(2, 3));
    REQUIRE(level.getPickups()[1].type == "gem");
}

TEST_CASE("A level saves the pickups it was given", "[Pickups]")
{
    Level level = levelFrom(levelPlacing({{"coin", glm::ivec2(2, 3)}}));

    REQUIRE(
        level.toLevelData().pickups == std::vector<PickupSpawnData>{{"coin", glm::ivec2(2, 3)}});
}

TEST_CASE("A placed pickup survives being written and read back", "[Pickups]")
{
    LevelData written = levelPlacing({{"coin", glm::ivec2(5, 6)}});

    std::string json;
    REQUIRE_FALSE(glz::write_json(written, json));

    LevelData read;
    REQUIRE_FALSE(glz::read_json(read, json));

    REQUIRE(read.pickups == written.pickups);
}

TEST_CASE("A pickup type survives being written and read back", "[Pickups]")
{
    PickupData written;
    written.sheet = Sheet{"textures/somewhere.png", glm::ivec2(24, 32)};
    written.animationData = FrameAnimationData{{7, 8}, 0.5f};
    written.size = glm::vec2(24.0f, 32.0f);
    written.scoreDelta = 3;

    std::string json;
    REQUIRE_FALSE(glz::write_json(written, json));

    PickupData read;
    REQUIRE_FALSE(glz::read_json(read, json));

    REQUIRE(read.sheet == written.sheet);
    REQUIRE(read.animationData.frames == written.animationData.frames);
    REQUIRE(read.animationData.frameDuration == written.animationData.frameDuration);
    REQUIRE(read.size == written.size);
    REQUIRE(read.scoreDelta == written.scoreDelta);
}

TEST_CASE("A pickup nothing is said about is worth nothing", "[Pickups]")
{
    REQUIRE(PickupData{}.scoreDelta == 0);
}

TEST_CASE("The shipped catalogue offers pickups to place", "[Pickups]")
{
    REQUIRE_FALSE(loadGameData().pickupData.empty());
}
