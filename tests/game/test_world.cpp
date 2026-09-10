#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/swing_ability_data.hpp"
#include <optional>
#include "game/level_data.hpp"
#include "helpers/tiles.hpp"
#include "helpers/actors.hpp"
#include <cstddef>
#include <string>
#include "game/world.hpp"
#include "game/noise.hpp"
#include "input/input_intentions.hpp"
#include "animations/frame_animation_data.hpp"
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
#include "actor/behaviors/idle_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
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

TEST_CASE("The player's signals reach the script by their hook names", "[World]")
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
    gameData.playerData.script.path = playerScript.string();
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
    gameData.playerData.script.path = playerScript.string();
    NpcData impostor = setupNpcData();
    impostor.script.path = npcScript.string();
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
    rat.script.path = ratScript.string();
    NpcData spider = setupNpcData();
    spider.script.path = spiderScript.string();
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
    rat.script.path = ratScript.string();
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
    rat.script.path = ratScript.string();
    gameData.npcData = {{"rat", rat}};
    ScriptedIntentions intentions;
    InputIntentions attacking;
    attacking.attack = std::string(SwingAttack);
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

TEST_CASE("A cue reaches the player's script by its name", "[World][Cues]")
{
    std::filesystem::path shared =
        std::filesystem::temp_directory_path() / "platformer_world_cue.lua";
    std::ofstream(shared) << "seen = {steps = 0}\n";
    std::filesystem::path playerScript =
        std::filesystem::temp_directory_path() / "platformer_world_cue_player.lua";
    std::ofstream(playerScript)
        << "return { onFootstep = function(who) seen.steps = seen.steps + 1; seen.alive = "
           "who:alive() end }\n";

    GameData gameData = aFloorWorldWithCoins();
    gameData.playerData.actorData.animationData.clips["idle"] =
        FrameAnimationData{{0, 1}, 0.05f, {{0, "onFootstep"}}};
    gameData.playerData.script.path = playerScript.string();
    LuaScriptSystem luaScriptSystem(shared.string());
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_cue");
    levels.write("floor.json", aFloorLevelPlacing({}));
    world.loadLevel(levels.pathOf("floor.json"));

    for (int step = 0; step < 50; ++step)
    {
        world.beginFrame();
        world.fixedUpdate(0.01f);
        world.postFixedUpdate();
    }

    sol::table seen = luaScriptSystem.getLua()["seen"];
    REQUIRE(seen["steps"].get<int>() >= 1);
    REQUIRE(seen["alive"].get<bool>());
}

TEST_CASE("A cue reaches the creature's own script, not the player's", "[World][Cues]")
{
    std::filesystem::path shared =
        std::filesystem::temp_directory_path() / "platformer_world_cue_npc.lua";
    std::ofstream(shared) << "seen = {}\n";
    std::filesystem::path playerScript =
        std::filesystem::temp_directory_path() / "platformer_world_cue_npc_player.lua";
    std::ofstream(playerScript) << "return { onSkitter = function(who) seen.wrong = true end }\n";
    std::filesystem::path ratScript =
        std::filesystem::temp_directory_path() / "platformer_world_cue_npc_rat.lua";
    std::ofstream(ratScript) << "return { onSkitter = function(rat) seen.who = rat:type() end }\n";

    GameData gameData = aFloorWorldWithCoins();
    gameData.playerData.script.path = playerScript.string();
    NpcData rat = setupNpcData();
    rat.actorData.animationData.clips["idle"] = FrameAnimationData{{0}, 0.05f, {{0, "onSkitter"}}};
    rat.script.path = ratScript.string();
    gameData.npcData = {{"rat", rat}};
    LuaScriptSystem luaScriptSystem(shared.string());
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_cue_npc");
    levels.write(
        "floor.json", aFloorLevelPlacing({spawnAt("rat", glm::ivec2(3, FloorLevelStanding))}));
    world.loadLevel(levels.pathOf("floor.json"));

    for (int step = 0; step < 50; ++step)
    {
        world.beginFrame();
        world.fixedUpdate(0.01f);
        world.postFixedUpdate();
    }

    sol::table seen = luaScriptSystem.getLua()["seen"];
    REQUIRE(seen["who"].get<std::string>() == "rat");
    REQUIRE_FALSE(seen["wrong"].valid());
}

TEST_CASE("The world hears the player land once, and the noise is gone the tick after", "[World]")
{
    GameData gameData = aFloorWorldWithCoins();
    TemporaryLevels levels("world_landing");
    levels.write("floor.json", aFloorLevelPlacing({}));
    LuaScriptSystem luaScriptSystem;
    ScriptedIntentions input;
    World world(gameData, input, luaScriptSystem);
    world.loadLevel(levels.pathOf("floor.json"));
    walkFor(world, 5);
    REQUIRE(world.noises().empty());

    InputIntentions jump;
    jump.jumpRequested = true;
    jump.jumpHeld = true;
    input.set(jump);
    walkFor(world, 1);
    input.set(InputIntentions{});

    int heardOn = -1;
    int heardAgain = 0;
    for (int frame = 0; frame < 120; ++frame)
    {
        walkFor(world, 1);
        if (world.noises().empty())
            continue;

        if (heardOn < 0)
        {
            heardOn = frame;
            REQUIRE(world.noises().size() == 1);
            REQUIRE(world.noises().front().kind == LandingNoise);
            REQUIRE(world.noises().front().at == world.getPlayer().feet());
        }
        else
            ++heardAgain;
    }

    REQUIRE(heardOn >= 0);
    REQUIRE(heardAgain == 0);
}

namespace
{
    NpcData aBoarThatSleepsUntilItHears(const std::string &script)
    {
        NpcData boar = setupNpcData();
        BehaviorStateData sleeping;
        sleeping.name = "sleep";
        sleeping.does = IdleBehaviorData{};
        BehaviorStateData charging;
        charging.name = "charge";
        charging.does = ChaseBehaviorData{};
        BehaviorTransitionData woken;
        woken.from = "sleep";
        woken.to = "charge";
        woken.when["heard"] = true;
        woken.when["near"] = true;
        BehaviorTransitionData tired;
        tired.from = "charge";
        tired.to = "sleep";
        tired.when["near"] = false;
        tired.after = 1.0f;
        boar.stateMachineBehaviorData =
            StateMachineBehaviorData{{sleeping, charging}, {woken, tired}};
        boar.facts["heard"] = false;
        boar.facts["near"] = false;
        boar.tuning["range"] = 200.0f;
        boar.script.path = script;
        return boar;
    }
}

TEST_CASE(
    "A creature hears through its script, and wakes the tick you land on its ground",
    "[World]")
{
    std::filesystem::path shared =
        std::filesystem::temp_directory_path() / "platformer_world_boar_shared.lua";
    std::ofstream(shared) << "seen = {ticks = 0}\n";
    std::filesystem::path boarScript =
        std::filesystem::temp_directory_path() / "platformer_world_boar.lua";
    std::ofstream(boarScript) << "return {\n"
                                 "  onNoise = function(boar, kind, at)\n"
                                 "    seen.kind = kind\n"
                                 "    seen.state = boar:state()\n"
                                 "    if kind == 'landing' and boar:onSameSurfaceAs(at) then\n"
                                 "      boar:event('heard', true)\n"
                                 "    end\n"
                                 "  end,\n"
                                 "  onTick = function(boar, dt)\n"
                                 "    seen.ticks = seen.ticks + 1\n"
                                 "    local threat = boar:threatFeet()\n"
                                 "    boar:fact('near', threat ~= nil and boar:distanceTo(threat) "
                                 "<= boar:tuning('range'))\n"
                                 "  end,\n"
                                 "}\n";

    GameData gameData = aFloorWorldWithCoins();
    gameData.npcData = {{"boar", aBoarThatSleepsUntilItHears(boarScript.string())}};
    ScriptedIntentions input;
    LuaScriptSystem luaScriptSystem(shared.string());
    World world(gameData, input, luaScriptSystem);
    TemporaryLevels levels("world_boar");
    levels.write(
        "floor.json", aFloorLevelPlacing({spawnAt("boar", glm::ivec2(6, FloorLevelStanding))}));
    world.loadLevel(levels.pathOf("floor.json"));
    const Npc &boar = *world.getLevel().getNpcs().front();
    sol::table seen = luaScriptSystem.getLua()["seen"];

    walkFor(world, 5);
    REQUIRE(boar.stateName() == "sleep");
    REQUIRE(seen["ticks"].get<int>() > 0);
    REQUIRE(std::get<bool>(boar.fact("near")));

    InputIntentions jump;
    jump.jumpRequested = true;
    jump.jumpHeld = true;
    input.set(jump);
    walkFor(world, 1);
    input.set(InputIntentions{});

    int wokeOn = -1;
    for (int frame = 0; frame < 120 && wokeOn < 0; ++frame)
    {
        walkFor(world, 1);
        if (boar.stateName() == "charge")
            wokeOn = frame;
    }

    REQUIRE(wokeOn >= 0);
    REQUIRE(seen["kind"].get<std::string>() == "landing");
    REQUIRE(seen["state"].get<std::string>() == "sleep");
    REQUIRE_FALSE(std::get<bool>(boar.fact("heard")));
}

namespace
{
    struct TwoWalkers
    {
        GameData gameData = aFloorWorldWithCoins();
        LuaScriptSystem luaScriptSystem;
        World world{gameData, noIntentions(), luaScriptSystem};
        TemporaryLevels levels{"world_cast"};

        TwoWalkers()
        {
            gameData.npcData = {{"rat", setupNpcData()}, {"spider", setupNpcData()}};
            LevelData levelData = aFloorLevelPlacing(
                {spawnAt("rat", glm::ivec2(3, FloorLevelStanding)),
                 spawnAt("spider", glm::ivec2(6, FloorLevelStanding))});
            levelData.pickups.push_back(
                PickupSpawnData{"coin", feetOf(glm::ivec2(8, FloorLevelStanding))});
            levels.write("floor.json", levelData);
            world.loadLevel(levels.pathOf("floor.json"));
        }

        TwoWalkers(const TwoWalkers &) = delete;
        TwoWalkers &operator=(const TwoWalkers &) = delete;

        const Npc &rat() const
        {
            return *world.getLevel().getNpcs()[0];
        }

        const Npc &spider() const
        {
            return *world.getLevel().getNpcs()[1];
        }
    };
}

TEST_CASE(
    "A cast change re-makes only the creatures whose kind changed, and leaves the rest walking",
    "[World]")
{
    TwoWalkers playing;
    walkFor(playing.world, 60);
    const Npc *ratBefore = &playing.rat();
    glm::vec2 ratWas = playing.rat().feet();
    glm::vec2 spiderWas = playing.spider().feet();
    REQUIRE(spiderWas != feetOf(glm::ivec2(6, FloorLevelStanding)));

    playing.gameData.npcData.at("spider").actorData.motionData.moveAbilityData->moveSpeed = 90.0f;
    playing.world.castChanged();

    REQUIRE(&playing.rat() == ratBefore);
    REQUIRE(playing.rat().feet() == ratWas);
    REQUIRE(playing.spider().feet() == feetOf(glm::ivec2(6, FloorLevelStanding)));
    REQUIRE(playing.spider().builtFrom().actorData.motionData.moveAbilityData->moveSpeed == 90.0f);
}

TEST_CASE(
    "A cast change leaves the player where they stand, re-made if their data changed",
    "[World]")
{
    TwoWalkers playing;
    glm::vec2 wandered = playing.world.getPlayer().feet() + glm::vec2(32.0f, 0.0f);
    playing.world.getPlayer().standAt(wandered);
    const Player *before = &playing.world.getPlayer();

    playing.world.castChanged();
    REQUIRE(&playing.world.getPlayer() == before);

    playing.gameData.playerData.fallFromHeightThreshold += 1.0f;
    playing.world.castChanged();

    REQUIRE(&playing.world.getPlayer() != before);
    REQUIRE(playing.world.getPlayer().feet() == wandered);
}

TEST_CASE("A cast change does not bring back a coin already taken", "[World]")
{
    TwoWalkers playing;
    playing.world.getPlayer().standAt(feetOf(glm::ivec2(8, FloorLevelStanding)));
    walkFor(playing.world, 2);
    REQUIRE(playing.world.getScore().total() == 1);
    REQUIRE(playing.world.getLevel().getPickups().empty());

    playing.gameData.pickupData.at("coin").scoreDelta = 5;
    playing.world.castChanged();

    REQUIRE(playing.world.getLevel().getPickups().empty());
}

TEST_CASE("A re-made creature is wired to its script, and the old one is forgotten", "[World]")
{
    std::filesystem::path shared =
        std::filesystem::temp_directory_path() / "platformer_world_recast_shared.lua";
    std::ofstream(shared) << "seen = {hurt = 0}\n";
    std::filesystem::path script =
        std::filesystem::temp_directory_path() / "platformer_world_recast.lua";
    std::ofstream(script) << "return { onHurt = function(who) seen.hurt = seen.hurt + 1 end }\n";

    GameData gameData = aFloorWorldWithCoins();
    NpcData rat = setupNpcData();
    rat.actorData.healthData = HealthData{3, 0.0f};
    rat.script = script.string();
    gameData.npcData = {{"rat", rat}};
    LuaScriptSystem luaScriptSystem(shared.string());
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_recast");
    levels.write(
        "floor.json", aFloorLevelPlacing({spawnAt("rat", glm::ivec2(3, FloorLevelStanding))}));
    world.loadLevel(levels.pathOf("floor.json"));

    gameData.npcData.at("rat").contactDamage = 2;
    world.castChanged();
    world.getLevel().getNpcs().front()->takeHit(Hit{1, glm::vec2(0.0f), false});

    REQUIRE(luaScriptSystem.getLua()["seen"]["hurt"].get<int>() == 1);
}

TEST_CASE("A coroutine the old creature started is dropped when it is re-made", "[World]")
{
    std::filesystem::path shared =
        std::filesystem::temp_directory_path() / "platformer_world_recast_co_shared.lua";
    std::ofstream(shared) << "waitSeconds = coroutine.yield\nseen = {}\n";
    std::filesystem::path script =
        std::filesystem::temp_directory_path() / "platformer_world_recast_co.lua";
    std::ofstream(script) << "return { onHurt = function(who)\n"
                             "  startCoroutine(function() waitSeconds(0.1) seen.woke = true end)\n"
                             "end }\n";

    GameData gameData = aFloorWorldWithCoins();
    NpcData rat = setupNpcData();
    rat.actorData.healthData = HealthData{3, 0.0f};
    rat.script = script.string();
    gameData.npcData = {{"rat", rat}};
    LuaScriptSystem luaScriptSystem(shared.string());
    World world(gameData, noIntentions(), luaScriptSystem);
    TemporaryLevels levels("world_recast_co");
    levels.write(
        "floor.json", aFloorLevelPlacing({spawnAt("rat", glm::ivec2(3, FloorLevelStanding))}));
    world.loadLevel(levels.pathOf("floor.json"));

    world.getLevel().getNpcs().front()->takeHit(Hit{1, glm::vec2(0.0f), false});
    gameData.npcData.at("rat").contactDamage = 2;
    world.castChanged();
    luaScriptSystem.update(0.2f);

    REQUIRE_FALSE(luaScriptSystem.getLua()["seen"]["woke"].valid());
}
