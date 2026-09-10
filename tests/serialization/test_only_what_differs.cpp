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
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
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
    level.nextLevel.path = "levels/level2.json";

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
        NpcSpawnData{"rat", glm::vec2(8.0f, 32.0f), std::nullopt},
        NpcSpawnData{"spider", glm::vec2(0.0f), std::nullopt}};

    REQUIRE(onlyWhatDiffers(npcs) == R"([{"type":"rat","feet":[8,32]},{"type":"spider"}])");
}

TEST_CASE("What was left out reads back as the default it was", "[OnlyWhatDiffers]")
{
    LevelData level;
    level.tileMapData.indices = {{0, 1}};
    level.tileMapData.tilePalette = "cave";
    level.npcs = {NpcSpawnData{"spider", glm::vec2(0.0f), std::nullopt}};

    LevelData back;
    REQUIRE_FALSE(glz::read_json(back, onlyWhatDiffers(level)));

    REQUIRE(back.tileMapData.indices == level.tileMapData.indices);
    REQUIRE(back.playerFeet == glm::vec2(0.0f));
    REQUIRE(back.nextLevel.path == level.nextLevel.path);
    REQUIRE(back.npcs.size() == 1);
    REQUIRE(back.npcs[0].feet == glm::vec2(0.0f));
    REQUIRE(back.pickups.empty());
}

TEST_CASE("A level file lays its grid out and keeps its leaves compact", "[OnlyWhatDiffers]")
{
    LevelData level;
    level.tileMapData.indices = {{0, 10}, {3, 0}};
    level.tileMapData.tilePalette = "cave";
    level.playerFeet = glm::vec2(8.0f, 16.0f);
    level.nextLevel.path = "levels/level2.json";

    REQUIRE(
        asFileText(level) == "{\n"
                             "    \"tileMapData\":{\n"
                             "        \"indices\":[[ 0,10],[ 3, 0]],\n"
                             "        \"tilePalette\":\"cave\"\n"
                             "    },\n"
                             "    \"playerFeet\":[8,16],\n"
                             "    \"nextLevel\":\"levels/level2.json\"\n"
                             "}");
}

TEST_CASE("A clip is written with its cues, and without them when it has none", "[OnlyWhatDiffers]")
{
    FrameAnimationData quiet{{1, 2}, 0.5f};
    FrameAnimationData cueing{{1, 2}, 0.5f, {{1, "onSwing"}}};

    REQUIRE(onlyWhatDiffers(quiet) == R"({"frames":[1,2],"frameDuration":0.5})");
    REQUIRE(
        onlyWhatDiffers(cueing) ==
        R"({"frames":[1,2],"frameDuration":0.5,"cues":[{"frame":1,"name":"onSwing"}]})");
}

TEST_CASE("A clip that does not loop says so, and a looping one says nothing", "[OnlyWhatDiffers]")
{
    FrameAnimationData once{{1, 2}, 0.5f};
    once.loops = false;

    REQUIRE(
        onlyWhatDiffers(FrameAnimationData{{1, 2}, 0.5f}) ==
        R"({"frames":[1,2],"frameDuration":0.5})");
    REQUIRE(onlyWhatDiffers(once) == R"({"frames":[1,2],"frameDuration":0.5,"loops":false})");
}

TEST_CASE(
    "A state is written with the kind of what it does, then only what differs inside",
    "[OnlyWhatDiffers]")
{
    BehaviorStateData chasing;
    chasing.name = "chase";
    ChaseBehaviorData chase;
    chase.standoff = 28.0f;
    chasing.does = chase;

    BehaviorStateData pouncing;
    pouncing.name = "pounce";
    pouncing.does = AttackBehaviorData{"pounce"};
    pouncing.cooldown = 2.0f;

    BehaviorStateData patrolling;
    patrolling.name = "patrol";
    patrolling.does = PatrolBehaviorData{};

    REQUIRE(
        onlyWhatDiffers(chasing) == R"({"name":"chase","does":{"kind":"chase","standoff":28}})");
    REQUIRE(
        onlyWhatDiffers(pouncing) ==
        R"({"name":"pounce","does":{"kind":"attack","with":"pounce"},"cooldown":2})");
    REQUIRE(onlyWhatDiffers(patrolling) == R"({"name":"patrol","does":{"kind":"patrol"}})");
}

TEST_CASE(
    "A state that does nothing writes no kind, and reads back doing nothing",
    "[OnlyWhatDiffers]")
{
    BehaviorStateData idling;
    idling.name = "idle";

    REQUIRE(onlyWhatDiffers(idling) == R"({"name":"idle"})");

    BehaviorStateData back;
    REQUIRE_FALSE(
        glz::read<glz::opts{.error_on_unknown_keys = true}>(back, onlyWhatDiffers(idling)));
    REQUIRE(back == idling);
}

TEST_CASE("What a state does survives a round trip through its kind", "[OnlyWhatDiffers]")
{
    BehaviorStateData chasing;
    chasing.name = "chase";
    ChaseBehaviorData chase;
    chase.standoff = 28.0f;
    chasing.does = chase;

    BehaviorStateData back;
    REQUIRE_FALSE(
        glz::read<glz::opts{.error_on_unknown_keys = true}>(back, onlyWhatDiffers(chasing)));
    REQUIRE(back == chasing);
}
