#include <catch2/catch_test_macros.hpp>
#include "game/level_data.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
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
#include "test_helpers/test_player_utils.hpp"

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

    glm::vec2 feet = world.getPlayer().getPhysicsBody().getAABB().bottomCenter();

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

TEST_CASE("Loading a level says so, for whoever is watching", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);

    int loaded = 0;
    world.onLevelLoaded.connect([&loaded] { loaded++; });

    world.loadLevel("levels/level6.json");

    REQUIRE(loaded == 1);

    world.loadLevel("levels/level1.json");

    REQUIRE(loaded == 2);
}

TEST_CASE("The player acts on the intentions the world was given", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    ScriptedIntentions intentions;
    World world(gameData, intentions, luaScriptSystem);

    world.loadLevel("levels/level6.json");
    float startX = world.getPlayer().getPosition().x;

    InputIntentions moveRight;
    moveRight.direction = {1.0f, 0.0f};
    intentions.set(moveRight);

    for (int step = 0; step < 30; step++)
    {
        world.preFixedUpdate();
        world.fixedUpdate(1.0f / 60.0f);
        world.postFixedUpdate();
    }

    REQUIRE(world.getPlayer().getPosition().x > startX);
}

TEST_CASE("An npc added to the level data is standing in the world it rebuilds", "[World]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);
    world.loadLevel("levels/level6.json");

    std::size_t before = world.getLevel().getNpcs().size();

    LevelData edited = world.getLevelData();
    edited.npcs.push_back(NpcSpawnData{"villager", edited.playerStart, {}});
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

    glm::vec2 spawnAt = spawnsIn(world.getLevel())[1].position;
    glm::vec2 movedTo{spawnAt.x - TestTileSize, spawnAt.y};

    LevelData edited = world.getLevelData();
    edited.npcs[1].position = movedTo;
    world.rebuildFrom(edited);

    glm::vec2 feet = world.getLevel().getNpcs()[1]->getPhysicsBody().getAABB().bottomCenter();

    REQUIRE(feet == movedTo);
}
