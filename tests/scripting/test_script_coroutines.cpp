#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include "scripting/lua_script_system.hpp"

namespace
{
    LuaScriptSystem scripted(const std::string &name, const std::string &body)
    {
        std::filesystem::path path = std::filesystem::temp_directory_path() / name;
        std::ofstream(path) << "waitSeconds = coroutine.yield\n" << body;
        return LuaScriptSystem(path.string());
    }

    int stepsOf(LuaScriptSystem &lua)
    {
        return lua.getLua()["steps"].get<int>();
    }

    const std::string CountingSteps = "steps = 0\n"
                                      "function onDeath()\n"
                                      "    startCoroutine(function()\n"
                                      "        steps = 1\n"
                                      "        waitSeconds(0.1)\n"
                                      "        steps = 2\n"
                                      "        waitSeconds(0.2)\n"
                                      "        steps = 3\n"
                                      "    end)\n"
                                      "end\n";
}

TEST_CASE("A coroutine runs up to its first wait the moment it starts", "[ScriptCoroutines]")
{
    LuaScriptSystem lua = scripted("platformer_co_start.lua", CountingSteps);

    lua.emit("onDeath");

    REQUIRE(stepsOf(lua) == 1);
}

TEST_CASE("A coroutine waits the seconds it asked for", "[ScriptCoroutines]")
{
    LuaScriptSystem lua = scripted("platformer_co_wait.lua", CountingSteps);
    lua.emit("onDeath");

    lua.update(0.05f);
    REQUIRE(stepsOf(lua) == 1);
    lua.update(0.05f);
    REQUIRE(stepsOf(lua) == 2);
}

TEST_CASE("A coroutine that waits again is held for its new wait", "[ScriptCoroutines]")
{
    LuaScriptSystem lua = scripted("platformer_co_again.lua", CountingSteps);
    lua.emit("onDeath");
    lua.update(0.1f);
    REQUIRE(stepsOf(lua) == 2);

    lua.update(0.15f);
    REQUIRE(stepsOf(lua) == 2);
    lua.update(0.05f);
    REQUIRE(stepsOf(lua) == 3);
}

TEST_CASE("One update resumes a coroutine at most once, however long it was", "[ScriptCoroutines]")
{
    LuaScriptSystem lua = scripted("platformer_co_once.lua", CountingSteps);
    lua.emit("onDeath");

    lua.update(5.0f);

    REQUIRE(stepsOf(lua) == 2);
}

TEST_CASE("A coroutine that finishes is dropped", "[ScriptCoroutines]")
{
    LuaScriptSystem lua = scripted(
        "platformer_co_done.lua",
        "steps = 0\n"
        "function onDeath()\n"
        "    startCoroutine(function()\n"
        "        waitSeconds(0.1)\n"
        "        steps = steps + 1\n"
        "    end)\n"
        "end\n");
    lua.emit("onDeath");

    lua.update(0.1f);
    lua.update(0.1f);
    lua.update(0.1f);

    REQUIRE(stepsOf(lua) == 1);
}

TEST_CASE("A yield with no wait ends the coroutine", "[ScriptCoroutines]")
{
    LuaScriptSystem lua = scripted(
        "platformer_co_bare_yield.lua",
        "steps = 0\n"
        "function onDeath()\n"
        "    startCoroutine(function()\n"
        "        coroutine.yield()\n"
        "        steps = steps + 1\n"
        "    end)\n"
        "end\n");
    lua.emit("onDeath");

    lua.update(1.0f);

    REQUIRE(stepsOf(lua) == 0);
}

TEST_CASE("A wait made of frames that add up to it is due, not a frame late", "[ScriptCoroutines]")
{
    LuaScriptSystem lua = scripted(
        "platformer_co_sum.lua",
        "steps = 0\n"
        "function onDeath()\n"
        "    startCoroutine(function() waitSeconds(0.3) steps = 1 end)\n"
        "end\n");
    lua.emit("onDeath");

    lua.update(0.1f);
    lua.update(0.2f);

    REQUIRE(stepsOf(lua) == 1);
}

TEST_CASE("Coroutines keep their own clocks", "[ScriptCoroutines]")
{
    LuaScriptSystem lua = scripted(
        "platformer_co_two.lua",
        "quick = 0\n"
        "slow = 0\n"
        "function onDeath()\n"
        "    startCoroutine(function() waitSeconds(0.1) quick = 1 end)\n"
        "    startCoroutine(function() waitSeconds(0.3) slow = 1 end)\n"
        "end\n");
    lua.emit("onDeath");

    lua.update(0.1f);
    REQUIRE(lua.getLua()["quick"].get<int>() == 1);
    REQUIRE(lua.getLua()["slow"].get<int>() == 0);
    lua.update(0.2f);
    REQUIRE(lua.getLua()["slow"].get<int>() == 1);
}
