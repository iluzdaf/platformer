#include <string>
#include <catch2/catch_test_macros.hpp>
#include "game/game_data.hpp"
#include "game/level_data.hpp"
#include "game/level_data_file.hpp"
#include "game/reloads.hpp"
#include "game/world.hpp"
#include "scripting/lua_script_system.hpp"
#include "helpers/temporary_levels.hpp"
#include "helpers/actors.hpp"
#include "player/player.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "ui/editor_ui.hpp"

namespace
{
    struct Playing
    {
        GameData gameData = loadGameData();
        LuaScriptSystem luaScriptSystem;
        World world{gameData, noIntentions(), luaScriptSystem};
        EditorUi editorUi;
        TemporaryLevels levels{"reloads"};
        std::string levelPath = levels.pathOf("level1.json");

        Playing()
        {
            levels.copyShipped("level1.json");

            world.loadLevel(levelPath);
            editorUi.levelTakesTheDisk(world.getLevelData(), levelPath);
        }

        Playing(const Playing &) = delete;
        Playing &operator=(const Playing &) = delete;

        void editInMemory()
        {
            LevelData edited = world.getLevelData();
            edited.nextLevel.path = "edited in memory";
            world.rebuildFrom(edited);
        }

        void changeOnDisk()
        {
            LevelData changed = readLevelData(levelPath);
            changed.nextLevel.path = "changed on disk";
            writeLevelData(changed, levelPath);
        }
    };
}

TEST_CASE("A level file that changed is followed while the level is clean", "[Reloads]")
{
    Playing playing;
    playing.changeOnDisk();

    reloads::levelChanged(playing.world, playing.editorUi, playing.levelPath);

    REQUIRE(playing.world.getLevelData().nextLevel.path == "changed on disk");
}

TEST_CASE("A level file written as it already was is not reloaded", "[Reloads]")
{
    Playing playing;
    glm::vec2 wandered = playing.world.getPlayer().feet() + glm::vec2(32.0f, 0.0f);
    playing.world.getPlayer().standAt(wandered);
    writeLevelData(playing.world.getLevelData(), playing.levelPath);

    reloads::levelChanged(playing.world, playing.editorUi, playing.levelPath);

    REQUIRE(playing.world.getPlayer().feet() == wandered);
}

TEST_CASE("A level file that changed leaves unsaved edits alone", "[Reloads]")
{
    Playing playing;
    playing.editInMemory();
    playing.changeOnDisk();

    reloads::levelChanged(playing.world, playing.editorUi, playing.levelPath);

    REQUIRE(playing.world.getLevelData().nextLevel.path == "edited in memory");
}

TEST_CASE("Game data changing reloads a clean level from disk", "[Reloads]")
{
    Playing playing;
    playing.changeOnDisk();

    reloads::gameDataChanged(playing.world, playing.editorUi, playing.gameData, playing.gameData);

    REQUIRE(playing.world.getLevelData().nextLevel.path == "changed on disk");
}

TEST_CASE("Game data changing rebuilds a level with unsaved edits from memory", "[Reloads]")
{
    Playing playing;
    playing.editInMemory();
    playing.changeOnDisk();

    reloads::gameDataChanged(playing.world, playing.editorUi, playing.gameData, playing.gameData);

    REQUIRE(playing.world.getLevelData().nextLevel.path == "edited in memory");
}

TEST_CASE("Game data changing takes the disk for sections that are clean", "[Reloads]")
{
    Playing playing;
    GameData onDisk = playing.gameData;
    onDisk.cameraData.zoom += 1.0f;

    reloads::gameDataChanged(playing.world, playing.editorUi, playing.gameData, onDisk);

    REQUIRE(playing.gameData.cameraData.zoom == onDisk.cameraData.zoom);
}

TEST_CASE(
    "Game data changing fires a section's command only when a clean section took a change",
    "[Reloads]")
{
    Playing playing;
    int settingsChanged = 0;
    int cameraChanged = 0;
    playing.editorUi.commands.onSettingsChanged.connect([&] { ++settingsChanged; });
    playing.editorUi.commands.onCameraChanged.connect([&] { ++cameraChanged; });

    reloads::gameDataChanged(playing.world, playing.editorUi, playing.gameData, playing.gameData);
    playing.editorUi.commands.drain();
    REQUIRE(settingsChanged == 0);
    REQUIRE(cameraChanged == 0);

    GameData onDisk = playing.gameData;
    onDisk.cameraData.zoom += 1.0f;
    reloads::gameDataChanged(playing.world, playing.editorUi, playing.gameData, onDisk);
    playing.editorUi.commands.drain();
    REQUIRE(settingsChanged == 0);
    REQUIRE(cameraChanged == 1);

    onDisk.settings.windowWidth += 10;
    reloads::gameDataChanged(playing.world, playing.editorUi, playing.gameData, onDisk);
    playing.editorUi.commands.drain();
    REQUIRE(settingsChanged == 1);
    REQUIRE(cameraChanged == 1);
}

TEST_CASE("Game data changing before any level is played loads the first", "[Reloads]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);
    EditorUi editorUi;

    reloads::gameDataChanged(world, editorUi, gameData, gameData);

    REQUIRE(world.getLevelPath() == gameData.levels.first.path);
}
