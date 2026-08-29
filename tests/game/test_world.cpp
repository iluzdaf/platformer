#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <string>
#include "game/world.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "input/input_manager.hpp"
#include "player/player.hpp"
#include "scripting/lua_script_system.hpp"

TEST_CASE("Loading a level fills the world with the level and its cast", "[World]")
{
    GameData gameData = loadGameData();
    InputManager inputManager;
    LuaScriptSystem luaScriptSystem;
    World world(gameData, inputManager, luaScriptSystem);

    world.loadLevel("levels/level6.json");

    REQUIRE(world.getLevelPath() == "levels/level6.json");
    REQUIRE(world.isPlaying("levels/level6.json"));
    REQUIRE_FALSE(world.isPlaying("levels/level1.json"));

    const Level &level = world.getLevel();
    REQUIRE(world.getNpcs().size() == level.getNpcs().size());
    REQUIRE(world.getActors().size() == world.getNpcs().size() + 1);
}

TEST_CASE("The player starts standing where the level says", "[World]")
{
    GameData gameData = loadGameData();
    InputManager inputManager;
    LuaScriptSystem luaScriptSystem;
    World world(gameData, inputManager, luaScriptSystem);

    world.loadLevel("levels/level6.json");

    glm::vec2 feet = world.getPlayer().getPhysicsBody().getAABB().bottomCenter();

    REQUIRE(
        feet == world.getLevel().getTileMap().tileToBottomCenterPosition(
                    world.getLevel().getPlayerStartTile()));
}

TEST_CASE("Rebuilding the player leaves the rest of the cast alone", "[World]")
{
    GameData gameData = loadGameData();
    InputManager inputManager;
    LuaScriptSystem luaScriptSystem;
    World world(gameData, inputManager, luaScriptSystem);

    world.loadLevel("levels/level6.json");
    std::size_t npcsBefore = world.getNpcs().size();
    const Player *before = &world.getPlayer();

    world.rebuildPlayer();

    REQUIRE(&world.getPlayer() != before);
    REQUIRE(world.getNpcs().size() == npcsBefore);
    REQUIRE(world.getActors().size() == npcsBefore + 1);
}

TEST_CASE("The player cannot be built before there is a level to stand on", "[World]")
{
    GameData gameData = loadGameData();
    InputManager inputManager;
    LuaScriptSystem luaScriptSystem;
    World world(gameData, inputManager, luaScriptSystem);

    REQUIRE(world.getLevelPath().empty());
    REQUIRE_FALSE(world.isPlaying("levels/level1.json"));
    REQUIRE_THROWS(world.rebuildPlayer());
}
