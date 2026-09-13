#include <filesystem>
#include <fstream>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <sol/sol.hpp>
#include "actor/behaviors/scripted_behavior.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "helpers/actor_facts.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"
#include "scripting/lua_script_system.hpp"
#include "scripting/lua_state_script.hpp"
#include "scripting/npc_hooks.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "game/level.hpp"
#include "helpers/ledge_and_wall.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"

namespace
{
    std::filesystem::path writeScript(const std::string &name, const std::string &contents)
    {
        std::filesystem::path path = std::filesystem::temp_directory_path() / name;
        std::ofstream file(path);
        file << contents;
        return path;
    }

    struct Scripted
    {
        LuaScriptSystem lua;
        LuaStateScript script;
        ScriptedBehavior behavior;

        Scripted(const std::string &name, const std::string &contents, const std::string &call)
            : lua(writeScript("platformer_lua_states_shared.lua", "seen = {}\n").string()),
              script(lua, "states", nullptr), behavior(ScriptedBehaviorData{call})
        {
            lua.use("states", writeScript(name, contents).string());
            behavior.scriptWith(&script);
        }
    };

    const std::string Counting = "return { states = { count = {\n"
                                 "    enter = function(self) self.ticks = 10 end,\n"
                                 "    decide = function(self)\n"
                                 "        self.ticks = (self.ticks or 0) + 1\n"
                                 "        self.decided = (self.decided or 0) + 1\n"
                                 "        local wants = Intentions.new()\n"
                                 "        wants.direction = vec2.new(self.ticks, self.decided)\n"
                                 "        return wants\n"
                                 "    end,\n"
                                 "    exit = function(self) seen.left = self.ticks end,\n"
                                 "} } }\n";
}

TEST_CASE("What a state's decide returns is what the creature asks for", "[LuaStateScript]")
{
    Scripted scripted(
        "platformer_lua_states_lean.lua",
        "return { states = { lean = { decide = function()\n"
        "    local wants = Intentions.new()\n"
        "    wants.direction = vec2.new(1, 0)\n"
        "    wants.attack = 'bite'\n"
        "    return wants\n"
        "end } } }\n",
        "lean");
    NavigationGraph navigationGraph = aWalkRun();

    InputIntentions asked =
        scripted.behavior.decide(0.01f, standingAt(navigationGraph, {0.0f, 192.0f}));

    REQUIRE(asked.direction.x == 1.0f);
    REQUIRE(asked.attack == "bite");
}

TEST_CASE("A state decides on the facts it is given", "[LuaStateScript]")
{
    Scripted scripted(
        "platformer_lua_states_facing.lua",
        "return { states = { face = { decide = function(self, npc, facts)\n"
        "    local wants = Intentions.new()\n"
        "    if facts.onGround and facts.threatFeet then\n"
        "        wants.direction = vec2.new(facts.threatFeet.x > facts.feet.x and 1 or -1, 0)\n"
        "    end\n"
        "    return wants\n"
        "end } } }\n",
        "face");
    NavigationGraph navigationGraph = aWalkRun();

    REQUIRE(
        scripted.behavior
            .decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(200.0f, 192.0f)))
            .direction.x == 1.0f);
    REQUIRE(
        scripted.behavior
            .decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(0.0f, 192.0f)))
            .direction.x == -1.0f);
    REQUIRE(
        scripted.behavior.decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f})).direction.x ==
        0.0f);
}

TEST_CASE("Each visit to a state starts with a fresh self, set up by enter", "[LuaStateScript]")
{
    Scripted scripted("platformer_lua_states_counting.lua", Counting, "count");
    NavigationGraph navigationGraph = aWalkRun();
    auto decide = [&]
    { return scripted.behavior.decide(0.01f, standingAt(navigationGraph, {0.0f, 192.0f})); };

    REQUIRE(decide().direction == glm::vec2(11.0f, 1.0f));
    REQUIRE(decide().direction == glm::vec2(12.0f, 2.0f));

    scripted.behavior.leave();
    scripted.behavior.reset();

    REQUIRE(scripted.lua.getLua()["seen"]["left"].get<int>() == 12);
    REQUIRE(decide().direction == glm::vec2(11.0f, 1.0f));
}

TEST_CASE("A state's self is forgotten along with whoever it belongs to", "[LuaStateScript]")
{
    LuaScriptSystem lua(writeScript("platformer_lua_states_owned.lua", "\n").string());
    lua.use("states", writeScript("platformer_lua_states_owners.lua", Counting).string());
    int mine = 0, theirs = 0;
    auto ticks = [&](const void *owner)
    {
        sol::object wanted = lua.callState("states", owner, "count", "decide");
        return wanted.as<InputIntentions>().direction.x;
    };
    lua.startStateAfresh(&mine, "count");
    lua.callState("states", &mine, "count", "enter");
    lua.callState("states", &theirs, "count", "enter");
    ticks(&mine);
    ticks(&theirs);

    lua.forget(&mine);

    REQUIRE(ticks(&mine) == 1.0f);
    REQUIRE(ticks(&theirs) == 12.0f);
}

TEST_CASE("A coroutine a state starts is forgotten along with its creature", "[LuaStateScript]")
{
    LuaScriptSystem lua(
        writeScript(
            "platformer_lua_states_waiting.lua", "waitSeconds = coroutine.yield\nseen = {}\n")
            .string());
    lua.use(
        "states",
        writeScript(
            "platformer_lua_states_sleepy.lua",
            "return { states = { doze = { decide = function()\n"
            "    startCoroutine(function() waitSeconds(0.1); seen.woke = true end)\n"
            "end } } }\n")
            .string());
    int mine = 0;
    lua.callState("states", &mine, "doze", "decide");

    lua.forget(&mine);
    lua.update(0.2f);

    REQUIRE_FALSE(lua.getLua()["seen"]["woke"].valid());
}

TEST_CASE("A state's script walks the walker it is lent", "[LuaStateScript]")
{
    Scripted scripted(
        "platformer_lua_states_walking.lua",
        "return { states = { wander = { decide = function(self, npc, facts, walker, dt)\n"
        "    if not walker:anchored() then return nil end\n"
        "    if walker:finished() then walker:routeTo(4) end\n"
        "    seen.from = walker:currentNode()\n"
        "    seen.to = walker:targetNode()\n"
        "    return walker:follow(dt)\n"
        "end } } }\n",
        "wander");
    NavigationGraph navigationGraph = aWalkRun();

    InputIntentions asked =
        scripted.behavior.decide(0.01f, standingAt(navigationGraph, {0.0f, 192.0f}));

    REQUIRE(asked.direction.x == 1.0f);
    REQUIRE(scripted.lua.getLua()["seen"]["from"].get<int>() == 0);
    REQUIRE(scripted.lua.getLua()["seen"]["to"].get<int>() == 1);
    REQUIRE(scripted.behavior.getTargetNodeId() == 1);
}

TEST_CASE("A decide that fails asks for nothing, and is reported", "[LuaStateScript]")
{
    Scripted scripted(
        "platformer_lua_states_failing.lua",
        "return { states = { fall = { decide = function() error('boom') end } } }\n",
        "fall");
    NavigationGraph navigationGraph = aWalkRun();

    InputIntentions asked;
    REQUIRE_NOTHROW(
        asked = scripted.behavior.decide(0.01f, standingAt(navigationGraph, {0.0f, 192.0f})));
    REQUIRE(asked.direction.x == 0.0f);
}

TEST_CASE("A state its script does not name asks for nothing", "[LuaStateScript]")
{
    Scripted scripted("platformer_lua_states_none.lua", "return {}\n", "missing");
    NavigationGraph navigationGraph = aWalkRun();

    REQUIRE(
        scripted.behavior.decide(0.01f, standingAt(navigationGraph, {0.0f, 192.0f})).direction.x ==
        0.0f);
}

TEST_CASE("A creature's scripted state is run by its own script", "[LuaStateScript]")
{
    NpcData walker = setupNpcData();
    BehaviorStateData walking;
    walking.name = "walk";
    walking.does = ScriptedBehaviorData{"walk"};
    walker.stateMachineBehaviorData = StateMachineBehaviorData{{walking}, {}};
    walker.script.path = "scripts/npcs/walker.lua";
    NpcSpawnData spawn = spawnAt("walker", ledge_and_wall::OnTheGround);
    Level level = levelWithALedgeAndAWall({spawn}, {{"walker", walker}});
    Npc npc(spawn, walker);
    LuaScriptSystem lua(
        writeScript("platformer_lua_states_npc_shared.lua", "seen = {}\n").string());
    lua.use(
        scriptOf("walker"),
        writeScript(
            "platformer_lua_states_walker.lua",
            "return { states = { walk = {\n"
            "    enter = function(self, npc) seen.enteredAs = npc:type() end,\n"
            "    decide = function()\n"
            "        local wants = Intentions.new()\n"
            "        wants.direction = vec2.new(1, 0)\n"
            "        return wants\n"
            "    end,\n"
            "} } }\n")
            .string());
    connectNpcHooks(lua, npc);
    float startedAt = npc.feet().x;

    stepNpc(npc, level, 30);

    REQUIRE(lua.getLua()["seen"]["enteredAs"].get<std::string>() == "walker");
    REQUIRE(npc.feet().x > startedAt);
}
