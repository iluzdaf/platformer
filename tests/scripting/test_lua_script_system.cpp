#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
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

    const std::string countingDeath =
        "deaths = 0\n"
        "function onDeath()\n"
        "    deaths = deaths + 1\n"
        "end\n";
}

TEST_CASE("A script that loads gives the game its handlers", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript("platformer_lua_ok.lua", countingDeath);

    LuaScriptSystem luaScriptSystem(path.string());
    luaScriptSystem.triggerDeath();

    REQUIRE(luaScriptSystem.getLua()["deaths"].get<int>() == 1);
}

TEST_CASE("A script with a syntax error is reported, not swallowed", "[LuaScriptSystem]")
{
    std::filesystem::path path = writeScript("platformer_lua_broken.lua", "this is not lua ===\n");

    REQUIRE_THROWS_WITH(
        LuaScriptSystem(path.string()),
        Catch::Matchers::ContainsSubstring("syntax error"));
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
    std::filesystem::path path = writeScript("platformer_lua_reload.lua", countingDeath);
    LuaScriptSystem luaScriptSystem(path.string());

    luaScriptSystem.triggerDeath();
    REQUIRE(luaScriptSystem.getLua()["deaths"].get<int>() == 1);

    writeScript("platformer_lua_reload.lua", "this is not lua ===\n");
    REQUIRE_THROWS_AS(luaScriptSystem.loadScripts(), std::exception);

    luaScriptSystem.triggerDeath();
    REQUIRE(luaScriptSystem.getLua()["deaths"].get<int>() == 2);
}
