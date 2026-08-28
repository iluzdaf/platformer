#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include "reloading/reloader.hpp"

namespace
{
    Reloader playing(const std::string &levelPath)
    {
        Reloader reloader;
        reloader.commands.isPlaying = [levelPath](const std::string &asked)
        { return asked == levelPath; };

        return reloader;
    }
}

TEST_CASE("A shader that changed is the shader rebuilt", "[Reloader]")
{
    Reloader reloader;
    std::vector<std::string> rebuilt;
    reloader.commands.onReloadShader.connect([&rebuilt](const std::string &path)
                                             { rebuilt.push_back(path); });

    reloader.shaderChanged("shaders/tile_set.fs");

    REQUIRE(rebuilt == std::vector<std::string>{"shaders/tile_set.fs"});
}

TEST_CASE("A texture that changed is the texture rebuilt", "[Reloader]")
{
    Reloader reloader;
    std::vector<std::string> rebuilt;
    reloader.commands.onReloadTexture.connect([&rebuilt](const std::string &path)
                                              { rebuilt.push_back(path); });

    reloader.textureChanged("textures/player.png");

    REQUIRE(rebuilt == std::vector<std::string>{"textures/player.png"});
}

TEST_CASE("Game data changing asks for everything again", "[Reloader]")
{
    Reloader reloader;
    int asked = 0;
    reloader.commands.onReload.connect([&asked] { ++asked; });

    reloader.gameDataChanged();

    REQUIRE(asked == 1);
}

TEST_CASE("A script changing asks only for scripts", "[Reloader]")
{
    Reloader reloader;
    int scripts = 0, everything = 0;
    reloader.commands.onReloadScripts.connect([&scripts] { ++scripts; });
    reloader.commands.onReload.connect([&everything] { ++everything; });

    reloader.scriptsChanged();

    REQUIRE(scripts == 1);
    REQUIRE(everything == 0);
}

TEST_CASE("The level being played is loaded again when it changes", "[Reloader]")
{
    Reloader reloader = playing("levels/level6.json");
    std::vector<std::string> loaded;
    reloader.commands.onLoadLevel.connect([&loaded](const std::string &path)
                                          { loaded.push_back(path); });

    reloader.levelChanged("levels/level6.json");

    REQUIRE(loaded == std::vector<std::string>{"levels/level6.json"});
}

TEST_CASE("A level nobody is playing is left alone", "[Reloader]")
{
    Reloader reloader = playing("levels/level6.json");
    std::vector<std::string> loaded;
    reloader.commands.onLoadLevel.connect([&loaded](const std::string &path)
                                          { loaded.push_back(path); });

    reloader.levelChanged("levels/level1.json");

    REQUIRE(loaded.empty());
}

TEST_CASE("A rebuild that throws is reported rather than escaping", "[Reloader]")
{
    Reloader reloader;
    reloader.commands.onReloadShader.connect([](const std::string &)
                                             { throw std::runtime_error("bad shader"); });

    REQUIRE_NOTHROW(reloader.shaderChanged("shaders/tile_set.fs"));
}

TEST_CASE("One bad asset does not stop the next from reloading", "[Reloader]")
{
    Reloader reloader;
    int textures = 0;
    reloader.commands.onReloadShader.connect([](const std::string &)
                                             { throw std::runtime_error("bad shader"); });
    reloader.commands.onReloadTexture.connect([&textures](const std::string &) { ++textures; });

    reloader.shaderChanged("shaders/tile_set.fs");
    reloader.textureChanged("textures/player.png");

    REQUIRE(textures == 1);
}
