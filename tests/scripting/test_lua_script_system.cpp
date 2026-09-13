#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>
#include "scripting/lua_script_system.hpp"

namespace
{
    std::filesystem::path writeScript(const std::string &name, const std::string &contents)
    {
        std::filesystem::path path = std::filesystem::temp_directory_path() / name;
        std::ofstream file(path);
        file << contents;
        return path;
    }

    const std::string CountingDeath = "deaths = 0\n"
                                      "function onDeath()\n"
                                      "    deaths = deaths + 1\n"
                                      "end\n";
}

TEST_CASE("A script that loads gives the game its handlers", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript("platformer_lua_ok.lua", CountingDeath);

    LuaScriptSystem luaScriptSystem(path.string());
    luaScriptSystem.emit("onDeath");

    REQUIRE(luaScriptSystem.getLua()["deaths"].get<int>() == 1);
}

TEST_CASE("A hook is handed whatever emit was given", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript(
        "platformer_lua_args.lua",
        "function onScored(points, by) scored = points; scorer = by end\n");
    LuaScriptSystem luaScriptSystem(path.string());

    luaScriptSystem.emit("onScored", 3, "coin");

    REQUIRE(luaScriptSystem.getLua()["scored"].get<int>() == 3);
    REQUIRE(luaScriptSystem.getLua()["scorer"].get<std::string>() == "coin");
}

TEST_CASE("A hook that fails is printed, and the next hook still runs", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript(
        "platformer_lua_failing_hook.lua",
        "hurts = 0\n"
        "function onDeath() error('boom') end\n"
        "function onHurt() hurts = hurts + 1 end\n");
    LuaScriptSystem luaScriptSystem(path.string());

    REQUIRE_NOTHROW(luaScriptSystem.emit("onDeath"));
    luaScriptSystem.emit("onHurt");

    REQUIRE(luaScriptSystem.getLua()["hurts"].get<int>() == 1);
}

TEST_CASE("A coroutine that fails is printed and dropped, not fatal", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript(
        "platformer_lua_failing_coroutine.lua",
        "waitSeconds = coroutine.yield\n"
        "resumed = 0\n"
        "function onDeath()\n"
        "    startCoroutine(function()\n"
        "        waitSeconds(0.1)\n"
        "        resumed = resumed + 1\n"
        "        error('boom')\n"
        "    end)\n"
        "end\n");
    LuaScriptSystem luaScriptSystem(path.string());
    luaScriptSystem.emit("onDeath");

    REQUIRE_NOTHROW(luaScriptSystem.update(0.2f));
    REQUIRE_NOTHROW(luaScriptSystem.update(0.2f));

    REQUIRE(luaScriptSystem.getLua()["resumed"].get<int>() == 1);
}

TEST_CASE("A coroutine that fails before its first wait is not kept either", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript(
        "platformer_lua_failing_start.lua",
        "function onDeath() startCoroutine(function() error('boom') end) end\n");
    LuaScriptSystem luaScriptSystem(path.string());

    REQUIRE_NOTHROW(luaScriptSystem.emit("onDeath"));
    REQUIRE_NOTHROW(luaScriptSystem.update(1.0f));
}

TEST_CASE("A hook nobody wrote is nothing to do", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript("platformer_lua_quiet.lua", CountingDeath);
    LuaScriptSystem luaScriptSystem(path.string());

    REQUIRE_NOTHROW(luaScriptSystem.emit("onSomethingNobodyHandles"));
    REQUIRE(luaScriptSystem.getLua()["deaths"].get<int>() == 0);
}

TEST_CASE("A hook that is not a function is nothing to do either", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript("platformer_lua_notfn.lua", "onDeath = 42\n");
    LuaScriptSystem luaScriptSystem(path.string());

    REQUIRE_NOTHROW(luaScriptSystem.emit("onDeath"));
}

TEST_CASE("A named script keeps its helpers to itself", "[LuaScriptSystem]")
{
    std::filesystem::path shared = writeScript("platformer_lua_shared.lua", "seen = {}\n");
    writeScript(
        "platformer_lua_one.lua",
        "local function mine() return \"one\" end\n"
        "helper = mine\n"
        "return { onAsked = function() seen.one = mine() end }\n");
    writeScript(
        "platformer_lua_two.lua",
        "return { onAsked = function() seen.two = (helper == nil) end }\n");

    LuaScriptSystem luaScriptSystem(shared.string());
    luaScriptSystem.use(
        "one", (std::filesystem::temp_directory_path() / "platformer_lua_one.lua").string());
    luaScriptSystem.use(
        "two", (std::filesystem::temp_directory_path() / "platformer_lua_two.lua").string());

    luaScriptSystem.emitTo("one", "onAsked", nullptr);
    luaScriptSystem.emitTo("two", "onAsked", nullptr);

    sol::table seen = luaScriptSystem.getLua()["seen"];
    REQUIRE(seen["one"].get<std::string>() == "one");
    REQUIRE(seen["two"].get<bool>());
    REQUIRE_FALSE(luaScriptSystem.getLua()["helper"].valid());
}

TEST_CASE("A named script reads what the game put in front of everyone", "[LuaScriptSystem]")
{
    std::filesystem::path shared =
        writeScript("platformer_lua_bound.lua", "greeting = \"hello\"\nseen = {}\n");
    writeScript(
        "platformer_lua_reader.lua", "return { onAsked = function() seen.said = greeting end }\n");

    LuaScriptSystem luaScriptSystem(shared.string());
    luaScriptSystem.use(
        "reader", (std::filesystem::temp_directory_path() / "platformer_lua_reader.lua").string());

    luaScriptSystem.emitTo("reader", "onAsked", nullptr);

    REQUIRE(luaScriptSystem.getLua()["seen"]["said"].get<std::string>() == "hello");
}

TEST_CASE("A script that names no handlers is reported, not fatal", "[LuaScriptSystem]")
{
    std::filesystem::path shared = writeScript("platformer_lua_nothandlers.lua", "\n");
    writeScript("platformer_lua_returnsnothing.lua", "local x = 1\n");

    LuaScriptSystem luaScriptSystem(shared.string());

    REQUIRE_NOTHROW(luaScriptSystem.use(
        "quiet",
        (std::filesystem::temp_directory_path() / "platformer_lua_returnsnothing.lua").string()));
    REQUIRE_NOTHROW(luaScriptSystem.emitTo("quiet", "onAsked", nullptr));
}

TEST_CASE("A hook nobody named on a script nobody used is nothing to do", "[LuaScriptSystem]")
{
    std::filesystem::path shared = writeScript("platformer_lua_nouse.lua", "\n");
    LuaScriptSystem luaScriptSystem(shared.string());

    REQUIRE_NOTHROW(luaScriptSystem.emitTo("nobody", "onAsked", nullptr));
}

TEST_CASE("A coroutine is forgotten along with whoever started it", "[LuaScriptSystem]")
{
    std::filesystem::path shared =
        writeScript("platformer_lua_owned.lua", "waitSeconds = coroutine.yield\nseen = {}\n");
    writeScript(
        "platformer_lua_owner.lua",
        "return { onAsked = function()\n"
        "    startCoroutine(function()\n"
        "        waitSeconds(0.1)\n"
        "        seen.woke = true\n"
        "    end)\n"
        "end }\n");

    LuaScriptSystem luaScriptSystem(shared.string());
    luaScriptSystem.use(
        "owner", (std::filesystem::temp_directory_path() / "platformer_lua_owner.lua").string());

    int mine = 0;
    luaScriptSystem.emitTo("owner", "onAsked", &mine);
    luaScriptSystem.forget(&mine);
    luaScriptSystem.update(0.2f);

    REQUIRE_FALSE(luaScriptSystem.getLua()["seen"]["woke"].valid());
}

TEST_CASE("Forgetting one owner leaves another's coroutine running", "[LuaScriptSystem]")
{
    std::filesystem::path shared =
        writeScript("platformer_lua_two_owners.lua", "waitSeconds = coroutine.yield\nseen = {}\n");
    writeScript(
        "platformer_lua_owner2.lua",
        "return { onAsked = function(who)\n"
        "    startCoroutine(function()\n"
        "        waitSeconds(0.1)\n"
        "        seen[who] = true\n"
        "    end)\n"
        "end }\n");

    LuaScriptSystem luaScriptSystem(shared.string());
    luaScriptSystem.use(
        "owner", (std::filesystem::temp_directory_path() / "platformer_lua_owner2.lua").string());

    int first = 0, second = 0;
    luaScriptSystem.emitTo("owner", "onAsked", &first, "first");
    luaScriptSystem.emitTo("owner", "onAsked", &second, "second");
    luaScriptSystem.forget(&first);
    luaScriptSystem.update(0.2f);

    sol::table seen = luaScriptSystem.getLua()["seen"];
    REQUIRE_FALSE(seen["first"].valid());
    REQUIRE(seen["second"].get<bool>());
}

TEST_CASE("A script with a syntax error is reported, not swallowed", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript("platformer_lua_broken.lua", "this is not lua ===\n");

    REQUIRE_THROWS_WITH(
        LuaScriptSystem(path.string()), Catch::Matchers::ContainsSubstring("syntax error"));
}

TEST_CASE("A script that is not there is reported", "[LuaScriptSystem]")
{
    std::filesystem::path path =
        std::filesystem::temp_directory_path() / "platformer_lua_absent.lua";
    std::filesystem::remove(path);

    REQUIRE_THROWS_AS(LuaScriptSystem(path.string()), std::exception);
}

TEST_CASE("A reload that fails leaves the handlers that were working", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript("platformer_lua_reload.lua", CountingDeath);
    LuaScriptSystem luaScriptSystem(path.string());

    luaScriptSystem.emit("onDeath");
    REQUIRE(luaScriptSystem.getLua()["deaths"].get<int>() == 1);

    writeScript("platformer_lua_reload.lua", "this is not lua ===\n");
    REQUIRE_THROWS_AS(luaScriptSystem.loadScripts(), std::exception);

    luaScriptSystem.emit("onDeath");
    REQUIRE(luaScriptSystem.getLua()["deaths"].get<int>() == 2);
}

TEST_CASE(
    "A script includes another by its path under assets, afresh each time",
    "[LuaScriptSystem]")
{
    std::filesystem::path shared = writeScript("platformer_lua_includes_shared.lua", "seen = {}\n");
    LuaScriptSystem luaScriptSystem(shared.string());

    luaScriptSystem.use(
        "includer",
        writeScript(
            "platformer_lua_includer.lua",
            "local patrol = include('scripts/steering/patrol.lua')\n"
            "seen.decides = type(patrol.decide) == 'function'\n"
            "seen.afresh = include('scripts/steering/patrol.lua') ~= patrol\n"
            "return {}\n")
            .string());

    REQUIRE(luaScriptSystem.errorsReported() == 0);
    REQUIRE(luaScriptSystem.getLua()["seen"]["decides"].get<bool>());
    REQUIRE(luaScriptSystem.getLua()["seen"]["afresh"].get<bool>());
}

TEST_CASE("An include that fails fails the script that asked for it", "[LuaScriptSystem]")
{
    std::filesystem::path shared = writeScript("platformer_lua_includes_nothing.lua", "\n");
    LuaScriptSystem luaScriptSystem(shared.string());

    luaScriptSystem.use(
        "includer",
        writeScript(
            "platformer_lua_bad_includer.lua",
            "local missing = include('scripts/nowhere.lua')\n"
            "return { onAsked = function() end }\n")
            .string());

    REQUIRE(luaScriptSystem.errorsReported() == 1);
}
