#include <catch2/catch_test_macros.hpp>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <glaze/glaze.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation_data.hpp"
#include "game/game_data.hpp"
#include "game/level_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "serialization/json_format.hpp"
#include "serialization/only_what_differs.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "tile_map/tile_data.hpp"

TEST_CASE("A tile that says nothing is written as nothing", "[OnlyWhatDiffers]")
{
    REQUIRE(onlyWhatDiffers(TileData{}) == "{}");
}

TEST_CASE("A tile is written with only the flags that are true", "[OnlyWhatDiffers]")
{
    TileData wall;
    wall.solid = wall.grippable = true;

    REQUIRE(onlyWhatDiffers(wall) == R"({"solid":true,"grippable":true})");
}

TEST_CASE("What a tile holds is written after its flags", "[OnlyWhatDiffers]")
{
    TileData spike;
    spike.deadly = true;
    spike.collider = TileColliderData{glm::vec2(0.0f, 12.0f), glm::vec2(16.0f, 4.0f)};
    spike.animationData = FrameAnimationData{{1, 2}, 0.5f};

    REQUIRE(
        onlyWhatDiffers(spike) ==
        R"({"deadly":true,"animationData":{"frames":[1,2],"frameDuration":0.5},)"
        R"("collider":{"offset":[0,12],"size":[16,4]}})");
}

TEST_CASE("A collider at its own defaults is written as an empty one", "[OnlyWhatDiffers]")
{
    TileData block;
    block.solid = true;
    block.collider = TileColliderData{};

    REQUIRE(onlyWhatDiffers(block) == R"({"solid":true,"collider":{}})");
}

struct WithAColliderAlready
{
    std::optional<TileColliderData> collider =
        TileColliderData{glm::vec2(1.0f, 1.0f), glm::vec2(2.0f, 2.0f)};
};

TEST_CASE("An optional held by default is compared to what it holds", "[OnlyWhatDiffers]")
{
    WithAColliderAlready untouched;
    REQUIRE(onlyWhatDiffers(untouched) == "{}");

    WithAColliderAlready resized;
    resized.collider->size = glm::vec2(16.0f, 16.0f);
    REQUIRE(onlyWhatDiffers(resized) == R"({"collider":{"size":[16,16]}})");

    WithAColliderAlready back;
    REQUIRE_FALSE(glz::read_json(back, onlyWhatDiffers(resized)));
    REQUIRE(back.collider == resized.collider);
}

TEST_CASE("Settings at their defaults are written as nothing", "[OnlyWhatDiffers]")
{
    GameSettingsData settings;
    REQUIRE(onlyWhatDiffers(settings) == "{}");

    settings.debug = true;
    REQUIRE(onlyWhatDiffers(settings) == R"({"debug":true})");
}

TEST_CASE("A level with nobody in it names no npcs or pickups", "[OnlyWhatDiffers]")
{
    LevelData level;
    level.tileMapData.indices = {{0, 1}};
    level.tileMapData.tilePalette = "cave";
    level.nextLevel = "levels/level2.json";

    REQUIRE(
        onlyWhatDiffers(level) ==
        R"({"tileMapData":{"indices":[[0,1]],"tilePalette":"cave"},"nextLevel":"levels/level2.json"})");
}

TEST_CASE("An entry in a map is written even when it says nothing", "[OnlyWhatDiffers]")
{
    std::map<std::string, TileData> tiles{{"0", TileData{}}, {"3", TileData{}}};

    REQUIRE(onlyWhatDiffers(tiles) == R"({"0":{},"3":{}})");
}

TEST_CASE("An item in a list keeps only what differs from a new one", "[OnlyWhatDiffers]")
{
    std::vector<NpcSpawnData> npcs{
        NpcSpawnData{"villager", glm::vec2(8.0f, 32.0f), std::nullopt},
        NpcSpawnData{"explorer", glm::vec2(0.0f), std::nullopt}};

    REQUIRE(
        onlyWhatDiffers(npcs) == R"([{"type":"villager","position":[8,32]},{"type":"explorer"}])");
}

TEST_CASE("What was left out reads back as the default it was", "[OnlyWhatDiffers]")
{
    LevelData level;
    level.tileMapData.indices = {{0, 1}};
    level.tileMapData.tilePalette = "cave";
    level.npcs = {NpcSpawnData{"explorer", glm::vec2(0.0f), std::nullopt}};

    LevelData back;
    REQUIRE_FALSE(glz::read_json(back, onlyWhatDiffers(level)));

    REQUIRE(back.tileMapData.indices == level.tileMapData.indices);
    REQUIRE(back.playerStart == glm::vec2(0.0f));
    REQUIRE(back.nextLevel == level.nextLevel);
    REQUIRE(back.npcs.size() == 1);
    REQUIRE(back.npcs[0].position == glm::vec2(0.0f));
    REQUIRE(back.pickups.empty());
}

TEST_CASE("A level file lays its grid out and keeps its leaves compact", "[OnlyWhatDiffers]")
{
    LevelData level;
    level.tileMapData.indices = {{0, 10}, {3, 0}};
    level.tileMapData.tilePalette = "cave";
    level.playerStart = glm::vec2(8.0f, 16.0f);
    level.nextLevel = "levels/level2.json";

    REQUIRE(
        asFileText(level) == "{\n"
                             "    \"tileMapData\":{\n"
                             "        \"indices\":[[ 0,10],[ 3, 0]],\n"
                             "        \"tilePalette\":\"cave\"\n"
                             "    },\n"
                             "    \"playerStart\":[8,16],\n"
                             "    \"nextLevel\":\"levels/level2.json\"\n"
                             "}");
}
