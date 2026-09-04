#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include "assets/asset_paths.hpp"
#include "reloading/reloader.hpp"

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
    Reloader reloader;
    std::vector<std::string> loaded;
    reloader.commands.onLoadLevel.connect([&loaded](const std::string &path)
                                          { loaded.push_back(path); });
    reloader.levelLoaded("levels/level6.json");

    reloader.levelChanged("levels/level6.json");

    REQUIRE(loaded == std::vector<std::string>{"levels/level6.json"});
}

TEST_CASE("A level nobody is playing is left alone", "[Reloader]")
{
    Reloader reloader;
    std::vector<std::string> loaded;
    reloader.commands.onLoadLevel.connect([&loaded](const std::string &path)
                                          { loaded.push_back(path); });
    reloader.levelLoaded("levels/level6.json");

    reloader.levelChanged("levels/level1.json");

    REQUIRE(loaded.empty());
}

TEST_CASE("A level changing before any is played is left alone", "[Reloader]")
{
    Reloader reloader;
    std::vector<std::string> loaded;
    reloader.commands.onLoadLevel.connect([&loaded](const std::string &path)
                                          { loaded.push_back(path); });

    reloader.levelChanged("levels/level1.json");

    REQUIRE(loaded.empty());
}

TEST_CASE("The reloader follows the level most recently played", "[Reloader]")
{
    Reloader reloader;
    std::vector<std::string> loaded;
    reloader.commands.onLoadLevel.connect([&loaded](const std::string &path)
                                          { loaded.push_back(path); });
    reloader.levelLoaded("levels/level6.json");
    reloader.levelLoaded("levels/level2.json");

    reloader.levelChanged("levels/level6.json");
    reloader.levelChanged("levels/level2.json");

    REQUIRE(loaded == std::vector<std::string>{"levels/level2.json"});
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

namespace
{
    struct Heard
    {
        std::vector<std::string> levels, shaders, textures;
        int everything = 0, scripts = 0;

        explicit Heard(Reloader &reloader)
        {
            reloader.commands.onLoadLevel.connect([this](const std::string &path)
                                                  { levels.push_back(path); });
            reloader.commands.onReloadShader.connect([this](const std::string &path)
                                                     { shaders.push_back(path); });
            reloader.commands.onReloadTexture.connect([this](const std::string &path)
                                                      { textures.push_back(path); });
            reloader.commands.onReload.connect([this] { ++everything; });
            reloader.commands.onReloadScripts.connect([this] { ++scripts; });
        }

        bool nothing() const
        {
            return levels.empty() && shaders.empty() && textures.empty() && everything == 0 &&
                   scripts == 0;
        }
    };
}

TEST_CASE("A png under textures is the texture reloaded", "[Reloader]")
{
    Reloader reloader;
    Heard heard(reloader);

    reloader.fileChanged("textures/player.png");

    REQUIRE(heard.textures == std::vector<std::string>{"textures/player.png"});
    REQUIRE(heard.shaders.empty());
}

TEST_CASE("A shader under shaders is the shader reloaded", "[Reloader]")
{
    Reloader reloader;
    Heard heard(reloader);

    reloader.fileChanged("shaders/tile_set.vs");
    reloader.fileChanged("shaders/tile_set.fs");

    REQUIRE(
        heard.shaders == std::vector<std::string>{"shaders/tile_set.vs", "shaders/tile_set.fs"});
}

TEST_CASE("A json under levels is the level reloaded, if it is being played", "[Reloader]")
{
    Reloader reloader;
    Heard heard(reloader);
    reloader.levelLoaded("levels/level1.json");

    reloader.fileChanged("levels/level1.json");
    reloader.fileChanged("levels/level2.json");

    REQUIRE(heard.levels == std::vector<std::string>{"levels/level1.json"});
    REQUIRE(heard.everything == 0);
}

TEST_CASE("Anything under scripts reloads the scripts", "[Reloader]")
{
    Reloader reloader;
    Heard heard(reloader);

    reloader.fileChanged("scripts/game_logic.lua");

    REQUIRE(heard.scripts == 1);
    REQUIRE(heard.everything == 0);
}

TEST_CASE("Every file loadGameData reads reloads everything", "[Reloader]")
{
    Reloader reloader;
    Heard heard(reloader);

    for (std::string_view named :
         {assets::GameSettings,
          assets::Camera,
          assets::Player,
          assets::Npcs,
          assets::Pickups,
          assets::TilePalettes,
          assets::LevelList})
        reloader.fileChanged(std::string(named));

    REQUIRE(heard.everything == 7);
    REQUIRE(heard.levels.empty());
}

TEST_CASE("A file nobody has heard of under data reloads everything too", "[Reloader]")
{
    Reloader reloader;
    Heard heard(reloader);

    reloader.fileChanged("data/weather.json");

    REQUIRE(heard.everything == 1);
}

TEST_CASE("A file nobody knows does nothing", "[Reloader]")
{
    Reloader reloader;
    Heard heard(reloader);
    reloader.levelLoaded("levels/level1.json");

    reloader.fileChanged("README.md");
    reloader.fileChanged("textures/notes.txt");
    reloader.fileChanged("player.png");
    reloader.fileChanged("levels.txt");
    reloader.fileChanged("levelsx/level1.json");

    REQUIRE(heard.nothing());
}
