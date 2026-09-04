#include <catch2/catch_test_macros.hpp>
#include "pickups/pickup.hpp"
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <map>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glaze/glaze.hpp>
#include "assets/sheet_data.hpp"
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
        levelData.playerStart = feetOf(glm::ivec2(0, 0));
        levelData.tileMapData.tilePalette = "default";
        levelData.tileMapData.indices = std::vector<std::vector<int>>(4, std::vector<int>(4, 0));
        levelData.pickups = pickups;
        return levelData;
    }

    const std::map<std::string, PickupData> &pickupCatalogue()
    {
        static const std::map<std::string, PickupData> kinds{
            {"coin", PickupData{}}, {"gem", PickupData{}}};
        return kinds;
    }

    Level levelFrom(const LevelData &levelData)
    {
        return Level(
            levelData,
            palettesFrom(getDefaultTileDataMap()),
            PlayerData(),
            shippedNpcData(),
            pickupCatalogue());
    }
}

TEST_CASE("A placed pickup survives being written and read back", "[Pickups]")
{
    LevelData written = levelPlacing({{"coin", middleOf(glm::ivec2(5, 6))}});

    std::string json;
    REQUIRE_FALSE(glz::write_json(written, json));

    LevelData read;

    read.playerStart = feetOf(glm::ivec2(0, 0));
    REQUIRE_FALSE(glz::read_json(read, json));

    REQUIRE(read.pickups == written.pickups);
}

TEST_CASE("A pickup type survives being written and read back", "[Pickups]")
{
    PickupData written;
    written.sheet = SheetData{"textures/somewhere.png", glm::ivec2(24, 32)};
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

TEST_CASE("A level naming a pickup that does not exist fails to load", "[Pickups]")
{
    REQUIRE_THROWS_WITH(
        levelFrom(levelPlacing({{"nobody", middleOf(glm::ivec2(1, 1))}})),
        Catch::Matchers::ContainsSubstring("nobody"));
}

TEST_CASE("A level advances the pickups it holds", "[Pickups]")
{
    std::map<std::string, PickupData> spinning{{"coin", PickupData{}}};
    spinning["coin"].animationData.frames = {0, 1, 2};
    spinning["coin"].animationData.frameDuration = 0.1f;

    Level level(
        levelPlacing({{"coin", middleOf(glm::ivec2(1, 1))}}),
        palettesFrom(getDefaultTileDataMap()),
        PlayerData(),
        shippedNpcData(),
        spinning);

    REQUIRE(level.getPickups().size() == 1);
    REQUIRE(level.getPickups().front().getCurrentFrame() == 0);

    level.update(0.15f);

    REQUIRE(level.getPickups().front().getCurrentFrame() == 1);
}

TEST_CASE("A pickup sits centred on where it was placed", "[Pickups]")
{
    glm::vec2 placedAt = middleOf(glm::ivec2(2, 3));
    Level level = levelFrom(levelPlacing({{"coin", placedAt}}));

    REQUIRE(level.getPickups().size() == 1);
    const Pickup &coin = level.getPickups().front();

    REQUIRE(coin.getPosition() + coin.getSize() * 0.5f == placedAt);
}
