#include <filesystem>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include "game/game_data.hpp"
#include "game/level_data.hpp"
#include "game/level_data_file.hpp"
#include "game/reloads.hpp"
#include "game/world.hpp"
#include "scripting/lua_script_system.hpp"
#include "test_helpers/asset_path.hpp"
#include "test_helpers/test_player_utils.hpp"
#include "ui/editor_ui.hpp"

namespace
{
    struct Playing
    {
        GameData gameData = loadGameData();
        LuaScriptSystem luaScriptSystem;
        World world{gameData, noIntentions(), luaScriptSystem};
        EditorUi editorUi;
        std::filesystem::path directory =
            std::filesystem::temp_directory_path() / "platformer_reloads";
        std::string levelPath = (directory / "level1.json").string();

        Playing()
        {
            std::filesystem::remove_all(directory);
            std::filesystem::create_directories(directory);
            std::filesystem::copy_file(assetPath("levels/level1.json"), levelPath);

            world.loadLevel(levelPath);
            editorUi.levelFollowsTheDisk(world.getLevelData(), levelPath);
        }

        ~Playing()
        {
            std::filesystem::remove_all(directory);
        }

        Playing(const Playing &) = delete;
        Playing &operator=(const Playing &) = delete;

        void editInMemory()
        {
            LevelData edited = world.getLevelData();
            edited.nextLevel = "edited in memory";
            world.rebuildFrom(edited);
        }

        void changeOnDisk()
        {
            LevelData changed = readLevelData(levelPath);
            changed.nextLevel = "changed on disk";
            writeLevelData(changed, levelPath);
        }
    };
}

TEST_CASE("A level file that changed is followed while the level is clean", "[Reloads]")
{
    Playing playing;
    playing.changeOnDisk();

    reloads::levelChanged(playing.world, playing.editorUi, playing.levelPath);

    REQUIRE(playing.world.getLevelData().nextLevel == "changed on disk");
}

TEST_CASE("A level file that changed leaves unsaved edits alone", "[Reloads]")
{
    Playing playing;
    playing.editInMemory();
    playing.changeOnDisk();

    reloads::levelChanged(playing.world, playing.editorUi, playing.levelPath);

    REQUIRE(playing.world.getLevelData().nextLevel == "edited in memory");
}

TEST_CASE("Game data changing reloads a clean level from disk", "[Reloads]")
{
    Playing playing;
    playing.changeOnDisk();

    reloads::gameDataChanged(playing.world, playing.editorUi, playing.gameData, playing.gameData);

    REQUIRE(playing.world.getLevelData().nextLevel == "changed on disk");
}

TEST_CASE("Game data changing rebuilds a level with unsaved edits from memory", "[Reloads]")
{
    Playing playing;
    playing.editInMemory();
    playing.changeOnDisk();

    reloads::gameDataChanged(playing.world, playing.editorUi, playing.gameData, playing.gameData);

    REQUIRE(playing.world.getLevelData().nextLevel == "edited in memory");
}

TEST_CASE("Game data changing takes the disk for sections that are clean", "[Reloads]")
{
    Playing playing;
    GameData onDisk = playing.gameData;
    onDisk.cameraData.zoom += 1.0f;

    reloads::gameDataChanged(playing.world, playing.editorUi, playing.gameData, onDisk);

    REQUIRE(playing.gameData.cameraData.zoom == onDisk.cameraData.zoom);
}

TEST_CASE("Game data changing before any level is played loads the first", "[Reloads]")
{
    GameData gameData = loadGameData();
    LuaScriptSystem luaScriptSystem;
    World world(gameData, noIntentions(), luaScriptSystem);
    EditorUi editorUi;

    reloads::gameDataChanged(world, editorUi, gameData, gameData);

    REQUIRE(world.getLevelPath() == gameData.levels.first);
}
