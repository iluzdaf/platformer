#ifndef SKIP_OPENGL_TESTS
#include "game/game_data.hpp"
#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>
#include "game/game.hpp"
#include "reloading/reloader.hpp"
#include "window/window.hpp"

TEST_CASE("Game tells the reloader which level it is playing", "[Game]")
{
    // Destroying a Window terminates GLFW, so this one outlives every test.
    static Window window(320, 240, "test");
    Reloader reloader;
    Game game(window, reloader);

    std::vector<std::string> loaded;
    reloader.commands.onLoadLevel.connect([&loaded](const std::string &path)
                                          { loaded.push_back(path); });
    std::string firstLevel = loadGameData().levels.first;

    SECTION("The level it started on is loaded again when it changes")
    {
        reloader.levelChanged(firstLevel);

        REQUIRE(loaded == std::vector<std::string>{firstLevel});
    }

    SECTION("A level it is not playing is left alone")
    {
        reloader.levelChanged("levels/level3.json");

        REQUIRE(loaded.empty());
    }

    SECTION("Being asked to load a level changes the level it is playing")
    {
        reloader.commands.onLoadLevel("levels/level2.json");
        loaded.clear();

        reloader.levelChanged(firstLevel);
        reloader.levelChanged("levels/level2.json");

        REQUIRE(loaded == std::vector<std::string>{"levels/level2.json"});
    }
}
#endif // SKIP_OPENGL_TESTS
