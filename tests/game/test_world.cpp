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
#include "helpers/npc_fixtures.hpp"
#include "actor/health.hpp"
#include "actor/health_data.hpp"
#include "npc/npc_data.hpp"
#include "actor/hit.hpp"
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
    edited.npcs.push_back(NpcSpawnData{"rat", edited.playerFeet, {}});
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
    edited.npcs.push_back(NpcSpawnData{"rat", edited.playerFeet, {}});
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
    broken.npcs.push_back(NpcSpawnData{"rat", glm::vec2(-100.0f, -100.0f), std::nullopt});

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
    glm::vec2 halfACoin = drawnSizeOf(gameData.pickupData.at("coin")) * 0.5f;
    auto coinWhoseLeftEdgeIsAt = [&](float x) { return glm::vec2(x, collider.top()) + halfACoin; };

    REQUIRE(scoreStandingBesideACoinAt(coinWhoseLeftEdgeIsAt(collider.right())) == 1);
    REQUIRE(scoreStandingBesideACoinAt(coinWhoseLeftEdgeIsAt(collider.right() + 1.0f)) == 0);
}

TEST_CASE("A hurt player reaches its own script, and a dead one too", "[World]")
{
    std::filesystem::path script =
        std::filesystem::temp_directory_path() / "platformer_world_hurt.lua";
    std::ofstream(script) << "seen = {hurts = 0, deaths = 0}\n";
    std::filesystem::path playerScript =
        std::filesystem::temp_directory_path() / "platformer_world_hurt_player.lua";
    std::ofstream(playerScript)
        << "return {\n"
           "  onHurt = function(who) seen.hurts = seen.hurts + 1; seen.alive = who:alive() end,\n"
           "  onDeath = function(who) seen.deaths = seen.deaths + 1; seen.dead = who:alive() end,\n"
           "}\n";
    GameData gameData = aFloorWorldWithCoins();
    gameData.playerData = playerDataWithHealth(2, 0.0f);
    gameData.playerData.script = playerScript.string();
    LuaScriptSystem luaScriptSystem(script.string());
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_hurt");
    levels.write("floor.json", aFloorLevelPlacing({}));
    world.loadLevel(levels.pathOf("floor.json"));

    world.getPlayer().takeHit(Hit{1, glm::vec2(0.0f), false});
    world.getPlayer().takeHit(Hit{1, glm::vec2(0.0f), false});

    sol::table seen = luaScriptSystem.getLua()["seen"];
    REQUIRE(seen["hurts"].get<int>() == 1);
    REQUIRE(seen["deaths"].get<int>() == 1);
    REQUIRE(seen["alive"].get<bool>());
    REQUIRE_FALSE(seen["dead"].get<bool>());
}

TEST_CASE("A creature named like the player does not answer for it", "[World]")
{
    std::filesystem::path shared =
        std::filesystem::temp_directory_path() / "platformer_world_clash.lua";
    std::ofstream(shared) << "seen = {}\n";
    std::filesystem::path playerScript =
        std::filesystem::temp_directory_path() / "platformer_world_clash_player.lua";
    std::ofstream(playerScript) << "return { onHurt = function(who) seen.who = \"player\" end }\n";
    std::filesystem::path npcScript =
        std::filesystem::temp_directory_path() / "platformer_world_clash_npc.lua";
    std::ofstream(npcScript) << "return { onHurt = function(who) seen.who = \"npc\" end }\n";

    GameData gameData = aFloorWorldWithCoins();
    gameData.playerData = playerDataWithHealth(3, 0.0f);
    gameData.playerData.script = playerScript.string();
    NpcData impostor = setupNpcData();
    impostor.script = npcScript.string();
    gameData.npcData = {{"player", impostor}};

    LuaScriptSystem luaScriptSystem(shared.string());
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_clash");
    levels.write(
        "floor.json", aFloorLevelPlacing({spawnAt("player", glm::ivec2(3, FloorLevelStanding))}));
    world.loadLevel(levels.pathOf("floor.json"));

    world.getPlayer().takeHit(Hit{1, glm::vec2(0.0f), false});

    REQUIRE(luaScriptSystem.getLua()["seen"]["who"].get<std::string>() == "player");
}

TEST_CASE("An npc's own script hears it hurt and killed, and no other npc's does", "[World]")
{
    std::filesystem::path shared =
        std::filesystem::temp_directory_path() / "platformer_world_shared.lua";
    std::ofstream(shared) << "seen = {}\n";

    std::filesystem::path ratScript =
        std::filesystem::temp_directory_path() / "platformer_world_rat.lua";
    std::ofstream(ratScript) << "return {\n"
                                "  onHurt = function(rat) seen.hurt = rat:type() end,\n"
                                "  onDied = function(rat) seen.dead = rat:type() end,\n"
                                "}\n";

    std::filesystem::path spiderScript =
        std::filesystem::temp_directory_path() / "platformer_world_spider.lua";
    std::ofstream(spiderScript) << "return { onHurt = function(s) seen.wrong = true end }\n";

    GameData gameData = aFloorWorldWithCoins();
    NpcData rat = setupNpcData();
    rat.actorData.healthData = HealthData{2, 0.0f};
    rat.script = ratScript.string();
    NpcData spider = setupNpcData();
    spider.script = spiderScript.string();
    gameData.npcData = {{"rat", rat}, {"spider", spider}};

    LuaScriptSystem luaScriptSystem(shared.string());
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_npc_hurt");
    levels.write(
        "floor.json", aFloorLevelPlacing({spawnAt("rat", glm::ivec2(3, FloorLevelStanding))}));
    world.loadLevel(levels.pathOf("floor.json"));
    Npc &npc = *world.getLevel().getNpcs().front();
    sol::table seen = luaScriptSystem.getLua()["seen"];

    npc.takeHit(Hit{1, glm::vec2(0.0f), false});
    REQUIRE(seen["hurt"].get<std::string>() == "rat");
    REQUIRE_FALSE(seen["dead"].valid());

    npc.takeHit(Hit{1, glm::vec2(0.0f), false});
    REQUIRE(seen["dead"].get<std::string>() == "rat");
    REQUIRE_FALSE(seen["wrong"].valid());
}

TEST_CASE("A coroutine an npc started is dropped when its level is rebuilt", "[World]")
{
    std::filesystem::path shared =
        std::filesystem::temp_directory_path() / "platformer_world_outlive.lua";
    std::ofstream(shared) << "waitSeconds = coroutine.yield\nseen = {}\n";
    std::filesystem::path ratScript =
        std::filesystem::temp_directory_path() / "platformer_world_outlive_rat.lua";
    std::ofstream(ratScript) << "return { onHurt = function(rat)\n"
                                "  startCoroutine(function()\n"
                                "    waitSeconds(0.1)\n"
                                "    seen.woke = rat:type()\n"
                                "  end)\n"
                                "end }\n";

    GameData gameData = aFloorWorldWithCoins();
    NpcData rat = setupNpcData();
    rat.actorData.healthData = HealthData{3, 0.0f};
    rat.script = ratScript.string();
    gameData.npcData = {{"rat", rat}};

    LuaScriptSystem luaScriptSystem(shared.string());
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_outlive");
    LevelData levelData = aFloorLevelPlacing({spawnAt("rat", glm::ivec2(3, FloorLevelStanding))});
    levels.write("floor.json", levelData);
    world.loadLevel(levels.pathOf("floor.json"));

    world.getLevel().getNpcs().front()->takeHit(Hit{1, glm::vec2(0.0f), false});
    world.rebuildFrom(levelData);
    luaScriptSystem.update(0.2f);

    REQUIRE_FALSE(luaScriptSystem.getLua()["seen"]["woke"].valid());
}

TEST_CASE("A swing that kills an npc reaches that npc's own script", "[World]")
{
    std::filesystem::path script =
        std::filesystem::temp_directory_path() / "platformer_world_swing.lua";
    std::ofstream(script) << "seen = {}\n";
    std::filesystem::path ratScript =
        std::filesystem::temp_directory_path() / "platformer_world_swing_rat.lua";
    std::ofstream(ratScript) << "return { onDied = function(rat) seen.dead = rat:type() end }\n";
    GameData gameData = aFloorWorldWithCoins();
    NpcData rat = setupNpcData();
    rat.script = ratScript.string();
    gameData.npcData = {{"rat", rat}};
    ScriptedIntentions intentions;
    InputIntentions attacking;
    attacking.attackRequested = true;
    intentions.set(attacking);
    LuaScriptSystem luaScriptSystem(script.string());
    World world(gameData, intentions, luaScriptSystem);
    TemporaryLevels levels("world_swing");
    levels.write(
        "floor.json", aFloorLevelPlacing({spawnAt("rat", glm::ivec2(2, FloorLevelStanding))}));
    world.loadLevel(levels.pathOf("floor.json"));

    walkFor(world, 12);

    REQUIRE_FALSE(world.getLevel().getNpcs().front()->alive());
    REQUIRE(luaScriptSystem.getLua()["seen"]["dead"].get<std::string>() == "rat");
}

TEST_CASE("Bumping into an npc that bites costs the player a point", "[World]")
{
    GameData gameData = aFloorWorldWithCoins();
    gameData.playerData = playerDataWithHealth(3, 1.0f);
    NpcData biter = setupNpcData();
    biter.contactDamage = 1;
    gameData.npcData = {{"biter", biter}};
    LevelData levelData = aFloorLevelPlacing({spawnAt("biter", glm::ivec2(1, FloorLevelStanding))});
    std::filesystem::path quiet =
        std::filesystem::temp_directory_path() / "platformer_world_quiet.lua";
    std::ofstream(quiet) << "";
    LuaScriptSystem luaScriptSystem(quiet.string());
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_bite");
    levels.write("floor.json", levelData);
    world.loadLevel(levels.pathOf("floor.json"));

    walkFor(world, 3);

    REQUIRE(world.getPlayer().health().points() == 2);
}
