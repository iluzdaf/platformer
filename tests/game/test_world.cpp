#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "game/level_data.hpp"
#include "helpers/tiles.hpp"
#include "helpers/actors.hpp"
#include <cstddef>
#include <string>
#include "game/world.hpp"
#include "input/input_intentions.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "player/player.hpp"
#include "npc/npc.hpp"
#include "npc/npc_spawn_data.hpp"
#include "physics/physics_body.hpp"
#include "physics/aabb.hpp"
#include "tile_map/tile_map.hpp"
#include "scripting/lua_script_system.hpp"
#include "timing/fixed_time_step.hpp"
#include "game/score.hpp"
#include "pickups/pickup_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "helpers/levels.hpp"
#include "helpers/palettes.hpp"
#include "helpers/temporary_levels.hpp"
#include "actor/hit.hpp"
#include "actor/health_data.hpp"
#include "actor/actor_data.hpp"
#include "player/player_data.hpp"
#include <filesystem>
#include <fstream>

namespace
{
    GameData aFloorWorldWithCoins()
    {
        GameData gameData;
        gameData.tilePalettes = theOnlyPalette(aPaletteWithASolidTile());
        gameData.playerData = playerDataWithEveryAbility();
        PickupData coin;
        coin.size = glm::vec2(16.0f);
        coin.scoreDelta = 1;
        gameData.pickupData = {{"coin", coin}};
        return gameData;
    }

    void walkFor(World &world, int frames)
    {
        FixedTimeStep timestepper;
        for (int frame = 0; frame < frames; ++frame)
        {
            world.beginFrame();
            timestepper.run(
                1.0f / 60.0f,
                [&](float dt)
                {
                    world.fixedUpdate(dt);
                    world.postFixedUpdate();
                });
        }
    }

    int scoreStandingBesideACoinAt(glm::vec2 position)
    {
        GameData gameData = aFloorWorldWithCoins();
        LevelData levelData = aFloorLevelPlacing({});
        levelData.pickups.push_back(PickupSpawnData{"coin", position});
        TemporaryLevels levels("world_coins");
        levels.write("floor.json", levelData);

        LuaScriptSystem luaScriptSystem;
        World world(gameData, noIntentions(), luaScriptSystem);
        world.loadLevel(levels.pathOf("floor.json"));
        walkFor(world, 10);

        return world.getScore().total();
    }
}

TEST_CASE("Loading a level fills the world with the level and its cast", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);

    world.loadLevel("levels/level6.json");

    REQUIRE(world.getLevelPath() == "levels/level6.json");

    const Level &level = world.getLevel();
    REQUIRE(world.getLevel().getNpcs().size() == spawnsIn(level).size());
}

TEST_CASE("The player starts standing where the level says", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);

    world.loadLevel("levels/level6.json");

    glm::vec2 feet = world.getPlayer().body().aabb().bottomCenter();

    REQUIRE(
        feet ==
        world.getLevel().getTileMap().feetOnTile(
            world.getLevel().getTileMap().tileUnderFeet(world.getLevel().getPlayerStart())));
}

TEST_CASE("Respawning the player leaves the rest of the cast alone", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);

    world.loadLevel("levels/level6.json");
    std::size_t npcsBefore = world.getLevel().getNpcs().size();
    const Player *before = &world.getPlayer();

    world.respawnPlayer();

    REQUIRE(&world.getPlayer() != before);
    REQUIRE(world.getLevel().getNpcs().size() == npcsBefore);
}

TEST_CASE("The player cannot be spawned before there is a level to stand on", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);

    REQUIRE(world.getLevelPath().empty());
    REQUIRE_THROWS(world.respawnPlayer());
}

TEST_CASE("Building a level says so, for whoever is watching", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);

    int built = 0;
    world.onLevelBuilt.connect([&built] { built++; });

    world.loadLevel("levels/level6.json");

    REQUIRE(built == 1);

    world.loadLevel("levels/level1.json");

    REQUIRE(built == 2);

    world.rebuildFrom(world.getLevelData());

    REQUIRE(built == 3);
}

TEST_CASE("The player acts on the intentions the world was given", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    ScriptedIntentions intentions;
    World world(gameData, intentions, luaScriptSystem);

    world.loadLevel("levels/level6.json");
    float startX = world.getPlayer().body().position().x;

    InputIntentions moveRight;
    moveRight.direction = {1.0f, 0.0f};
    intentions.set(moveRight);

    walkFor(world, 30);

    REQUIRE(world.getPlayer().body().position().x > startX);
}

TEST_CASE("An npc added to the level data is standing in the world it rebuilds", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);
    world.loadLevel("levels/level6.json");

    std::size_t before = world.getLevel().getNpcs().size();

    LevelData edited = world.getLevelData();
    edited.npcs.push_back(NpcSpawnData{"villager", edited.playerFeet, {}});
    world.rebuildFrom(edited);

    REQUIRE(world.getLevel().getNpcs().size() == before + 1);
}

TEST_CASE("An npc removed from the level data is gone from the world it rebuilds", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);
    world.loadLevel("levels/level6.json");

    std::size_t before = world.getLevel().getNpcs().size();

    LevelData edited = world.getLevelData();
    edited.npcs.erase(edited.npcs.begin());
    world.rebuildFrom(edited);

    REQUIRE(world.getLevel().getNpcs().size() == before - 1);
}

TEST_CASE("A spawn moved in the level data is where the npc stands", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);
    world.loadLevel("levels/level6.json");

    glm::vec2 spawnAt = spawnsIn(world.getLevel())[1].feet;
    glm::vec2 movedTo{spawnAt.x - TestTileSize, spawnAt.y};

    LevelData edited = world.getLevelData();
    edited.npcs[1].feet = movedTo;
    world.rebuildFrom(edited);

    glm::vec2 feet = world.getLevel().getNpcs()[1]->body().aabb().bottomCenter();

    REQUIRE(feet == movedTo);
}

TEST_CASE("Rebuilding from edited data leaves the player where it walked to", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    ScriptedIntentions intentions;
    World world(gameData, intentions, luaScriptSystem);
    world.loadLevel("levels/level6.json");

    InputIntentions moveRight;
    moveRight.direction = {1.0f, 0.0f};
    intentions.set(moveRight);
    walkFor(world, 30);
    const Player *before = &world.getPlayer();
    glm::vec2 walkedTo = world.getPlayer().body().position();
    REQUIRE(walkedTo != world.getLevel().getPlayerStart());

    LevelData edited = world.getLevelData();
    edited.npcs.push_back(NpcSpawnData{"villager", edited.playerFeet, {}});
    world.rebuildFrom(edited);

    REQUIRE(&world.getPlayer() == before);
    REQUIRE(world.getPlayer().body().position() == walkedTo);
}

TEST_CASE("A moved player start does not move the player until it respawns", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);
    world.loadLevel("levels/level6.json");
    glm::vec2 stoodAt = world.getPlayer().body().position();

    LevelData edited = world.getLevelData();
    edited.playerFeet = spawnsIn(world.getLevel())[0].feet;
    REQUIRE(edited.playerFeet != world.getLevel().getPlayerStart());
    world.rebuildFrom(edited);

    REQUIRE(world.getPlayer().body().position() == stoodAt);

    world.respawnPlayer();

    const TileMap &tileMap = world.getLevel().getTileMap();
    REQUIRE(
        world.getPlayer().body().aabb().bottomCenter() ==
        tileMap.feetOnTile(tileMap.tileUnderFeet(world.getLevel().getPlayerStart())));
}

TEST_CASE("A rebuild with a shift moves the player once the level stands", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    ScriptedIntentions intentions;
    World world(gameData, intentions, luaScriptSystem);
    world.loadLevel("levels/level6.json");

    glm::vec2 stoodAt = world.getPlayer().body().position();
    glm::vec2 npcAt = world.getLevel().getNpcs().front()->body().position();

    world.rebuildFrom(world.getLevelData(), glm::vec2(16.0f, 0.0f));

    REQUIRE(world.getPlayer().body().position() == stoodAt + glm::vec2(16.0f, 0.0f));
    REQUIRE(world.getLevel().getNpcs().front()->body().position() == npcAt);
}

TEST_CASE("A rebuild that cannot be built leaves the world as it was", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    ScriptedIntentions intentions;
    World world(gameData, intentions, luaScriptSystem);
    world.loadLevel("levels/level6.json");
    const Level *before = &world.getLevel();
    LevelData wasPlaying = world.getLevelData();
    glm::vec2 stoodAt = world.getPlayer().body().position();

    LevelData broken = wasPlaying;
    broken.npcs.push_back(NpcSpawnData{"villager", glm::vec2(-100.0f, -100.0f), std::nullopt});

    REQUIRE_THROWS(world.rebuildFrom(broken, glm::vec2(16.0f, 0.0f)));
    REQUIRE(&world.getLevel() == before);
    REQUIRE(world.getLevelData().npcs.size() == wasPlaying.npcs.size());
    REQUIRE(world.getPlayer().body().position() == stoodAt);
}

TEST_CASE("A pickup the player's collider only grazes is taken", "[World]")
{
    GameData gameData = aFloorWorldWithCoins();
    LevelData levelData = aFloorLevelPlacing({});
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_reach");
    levels.write("floor.json", levelData);
    world.loadLevel(levels.pathOf("floor.json"));
    AABB collider = world.getPlayer().body().aabb();
    glm::vec2 halfACoin = gameData.pickupData.at("coin").size * 0.5f;
    auto coinWhoseLeftEdgeIsAt = [&](float x) { return glm::vec2(x, collider.top()) + halfACoin; };

    REQUIRE(scoreStandingBesideACoinAt(coinWhoseLeftEdgeIsAt(collider.right())) == 1);
    REQUIRE(scoreStandingBesideACoinAt(coinWhoseLeftEdgeIsAt(collider.right() + 1.0f)) == 0);
}

TEST_CASE("A hurt player reaches the script's onHurt, and a dead one its onDeath", "[World]")
{
    std::filesystem::path script =
        std::filesystem::temp_directory_path() / "platformer_world_hurt.lua";
    std::ofstream(script) << "hurts = 0\ndeaths = 0\n"
                             "function onHurt() hurts = hurts + 1 end\n"
                             "function onDeath() deaths = deaths + 1 end\n";
    GameData gameData = aFloorWorldWithCoins();
    gameData.playerData.actorData.healthData = HealthData{2, 0.0f};
    LuaScriptSystem luaScriptSystem(script.string());
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_hurt");
    levels.write("floor.json", aFloorLevelPlacing({}));
    world.loadLevel(levels.pathOf("floor.json"));

    world.getPlayer().takeHit(Hit{1, glm::vec2(0.0f), false});
    world.getPlayer().takeHit(Hit{1, glm::vec2(0.0f), false});

    REQUIRE(luaScriptSystem.getLua()["hurts"].get<int>() == 1);
    REQUIRE(luaScriptSystem.getLua()["deaths"].get<int>() == 1);
}
