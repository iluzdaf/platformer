#ifndef SKIP_OPENGL_TESTS
#include <catch2/catch_test_macros.hpp>
#include <string>
#include "game/game.hpp"
#include "game/levels.hpp"
#include "reloading/reload_commands.hpp"
#include "window/window.hpp"
#include "test_helpers/asset_path.hpp"

TEST_CASE("Game answers the reload commands it was given", "[Game]")
{
    // Destroying a Window terminates GLFW, so this one outlives every test.
    static Window window(320, 240, "test");
    ReloadCommands commands;
    Game game(window, commands);

    std::string firstLevel = Levels(assetPath("levels.json")).getFirst();

    SECTION("The level it started on is the level it is playing")
    {
        REQUIRE(commands.isPlaying);
        REQUIRE(commands.isPlaying(firstLevel));
    }

    SECTION("A level it is not playing is not the level it is playing")
    {
        REQUIRE_FALSE(commands.isPlaying("levels/level3.json"));
    }

    SECTION("Being asked to load a level changes the level it is playing")
    {
        commands.onLoadLevel("levels/level2.json");

        REQUIRE(commands.isPlaying("levels/level2.json"));
        REQUIRE_FALSE(commands.isPlaying(firstLevel));
    }
}
#endif // SKIP_OPENGL_TESTS
