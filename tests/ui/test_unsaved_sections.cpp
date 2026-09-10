#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <filesystem>
#include <string>
#include "game/levels_data.hpp"
#include <tuple>
#include "game/level_data.hpp"
#include "cameras/camera2d.hpp"
#include "cameras/camera2d_data.hpp"
#include "game/game_data.hpp"
#include "helpers/headless_imgui.hpp"
#include "game/level.hpp"
#include "game/level_data_file.hpp"
#include "helpers/asset_path.hpp"
#include "helpers/temporary_levels.hpp"
#include "ui/camera_ui.hpp"
#include "ui/levels_ui.hpp"
#include "ui/editor_commands.hpp"
#include "ui/level_ui.hpp"

namespace
{
    struct ACopyOfLevelOne
    {
        TemporaryLevels levels{"level_on_disk"};
        std::string path = (levels.copyShipped("level1.json"), levels.pathOf("level1.json"));
    };

    void drawCameraOnce(HeadlessImGui &gui, CameraUi &cameraUi, GameData &gameData)
    {
        Camera2D camera(gameData.cameraData, 800, 600);
        EditorCommands commands;
        gui.frame([&] { cameraUi.draw(gameData, camera, commands); });
    }
}

TEST_CASE("A section has nothing unsaved before anything is edited", "[UnsavedSections]")
{
    HeadlessImGui gui;
    CameraUi cameraUi;
    GameData gameData;

    drawCameraOnce(gui, cameraUi, gameData);

    REQUIRE_FALSE(cameraUi.unsavedSince(gameData));
}

TEST_CASE("A section reports unsaved once its data changes", "[UnsavedSections]")
{
    CameraUi cameraUi;
    GameData gameData;

    REQUIRE_FALSE(cameraUi.unsavedSince(gameData));
    gameData.cameraData.zoom += 1.0f;

    REQUIRE(cameraUi.unsavedSince(gameData));
}

TEST_CASE("The first look at a section is what it is compared against", "[UnsavedSections]")
{
    CameraUi cameraUi;
    GameData gameData;
    gameData.cameraData.zoom += 1.0f;

    REQUIRE_FALSE(cameraUi.unsavedSince(gameData));
}

TEST_CASE("The levels section has nothing unsaved when it is first drawn", "[UnsavedSections]")
{
    HeadlessImGui gui;
    LevelsUi levelsUi;
    GameData gameData = loadGameData();
    LevelsData levels = gameData.levels;
    std::string levelPath = assetPath(levels.first.path);
    Level level(
        readLevelData(levelPath),
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);
    EditorCommands commands;

    gui.frame([&] { levelsUi.draw(levels, levelPath, commands, false); });

    REQUIRE_FALSE(levelsUi.unsavedSince(levels));
}

TEST_CASE("The levels section reports unsaved once the first level changes", "[UnsavedSections]")
{
    HeadlessImGui gui;
    LevelsUi levelsUi;
    GameData gameData = loadGameData();
    LevelsData levels = gameData.levels;
    std::string levelPath = assetPath(levels.first.path);
    Level level(
        readLevelData(levelPath),
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);
    EditorCommands commands;

    REQUIRE_FALSE(levelsUi.unsavedSince(levels));
    levels.first.path = "levels/level3.json";

    REQUIRE(levelsUi.unsavedSince(levels));
}

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/armed.hpp"
#include "ui/level_ui.hpp"
#include "ui/mouse_on_the_map.hpp"

TEST_CASE("A level edited with the inspector shut still reports unsaved", "[UnsavedSections]")
{
    LevelUi levelUi;
    GameData gameData = loadGameData();
    std::string levelPath = assetPath("levels/level1.json");
    LevelData levelData = readLevelData(levelPath);
    Level level(
        levelData,
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);

    std::optional<Armed> armed;
    EditorCommands commands;
    MouseOnTheMap still{true, glm::vec2(0.0f), false, false};

    levelUi.update(still, level, levelData, levelPath, armed, commands);
    REQUIRE_FALSE(levelUi.unsavedSince(levelData, levelPath));

    LevelData edited = levelData;
    edited.nextLevel.path = "levels/level3.json";

    REQUIRE(levelUi.unsavedSince(edited, levelPath));
}

TEST_CASE("A level painted from another section reports unsaved", "[UnsavedSections]")
{
    LevelUi levelUi;
    GameData gameData = loadGameData();
    std::string levelPath = assetPath("levels/level1.json");
    LevelData levelData = readLevelData(levelPath);
    Level level(
        levelData,
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);
    levelData.tileMapData = level.getTileMap().toTileMapData();

    std::optional<Armed> armed = PaintTile{5};
    EditorCommands commands;
    std::optional<LevelData> painted;
    std::ignore =
        commands.onLevelEdited.connect([&painted](const LevelData &now) { painted = now; });
    MouseOnTheMap mouse{false, level.getTileMap().feetOnTile(glm::ivec2(2, 2)), true, false};

    levelUi.update(mouse, level, levelData, levelPath, armed, commands);
    REQUIRE_FALSE(levelUi.unsavedSince(levelData, levelPath));

    commands.drain();
    REQUIRE(painted);
    REQUIRE(levelUi.unsavedSince(*painted, levelPath));
}

TEST_CASE("Reverting the levels section puts the first level back", "[UnsavedSections]")
{
    LevelsUi levelsUi;
    LevelsData levels = loadGameData().levels;
    std::string was = levels.first.path;

    REQUIRE_FALSE(levelsUi.unsavedSince(levels));

    levels.first.path = "levels/level3.json";
    REQUIRE(levelsUi.unsavedSince(levels));

    levelsUi.revert(levels);

    REQUIRE(levels.first.path == was);
    REQUIRE_FALSE(levelsUi.unsavedSince(levels));
}

TEST_CASE("A clean level whose file is as it knows it has nothing to take", "[UnsavedSections]")
{
    LevelUi levelUi;
    ACopyOfLevelOne copy;
    const std::string &levelPath = copy.path;
    LevelData levelData = readLevelData(levelPath);
    REQUIRE_FALSE(levelUi.unsavedSince(levelData, levelPath));

    REQUIRE_FALSE(levelUi.takesTheDisk(levelData, levelPath));
    REQUIRE_FALSE(levelUi.unsavedSince(levelData, levelPath));
}

TEST_CASE("A clean level takes the disk once its file changes", "[UnsavedSections]")
{
    LevelUi levelUi;
    ACopyOfLevelOne copy;
    const std::string &levelPath = copy.path;
    LevelData levelData = readLevelData(levelPath);
    REQUIRE_FALSE(levelUi.unsavedSince(levelData, levelPath));

    LevelData changedOnDisk = levelData;
    changedOnDisk.playerFeet.x += 16.0f;
    writeLevelData(changedOnDisk, levelPath);

    REQUIRE(levelUi.takesTheDisk(levelData, levelPath));
}

TEST_CASE("A level with unsaved edits is kept and stays unsaved", "[UnsavedSections]")
{
    LevelUi levelUi;
    ACopyOfLevelOne copy;
    const std::string &levelPath = copy.path;
    LevelData levelData = readLevelData(levelPath);
    REQUIRE_FALSE(levelUi.unsavedSince(levelData, levelPath));

    LevelData edited = levelData;
    edited.playerFeet.x += 16.0f;

    REQUIRE_FALSE(levelUi.takesTheDisk(edited, levelPath));
    REQUIRE(levelUi.unsavedSince(edited, levelPath));
}

TEST_CASE(
    "A level kept through a reload is compared against what is on disk now",
    "[UnsavedSections]")
{
    LevelUi levelUi;
    ACopyOfLevelOne copy;
    const std::string &levelPath = copy.path;
    LevelData levelData = readLevelData(levelPath);
    REQUIRE_FALSE(levelUi.unsavedSince(levelData, levelPath));

    LevelData edited = levelData;
    edited.playerFeet.x += 16.0f;
    writeLevelData(edited, levelPath);

    REQUIRE_FALSE(levelUi.takesTheDisk(edited, levelPath));
    REQUIRE_FALSE(levelUi.unsavedSince(edited, levelPath));
}

TEST_CASE("A level that took the disk is compared against what it loaded", "[UnsavedSections]")
{
    LevelUi levelUi;
    ACopyOfLevelOne copy;
    const std::string &levelPath = copy.path;
    LevelData levelData = readLevelData(levelPath);
    REQUIRE_FALSE(levelUi.unsavedSince(levelData, levelPath));

    LevelData changedOnDisk = levelData;
    changedOnDisk.playerFeet.x += 16.0f;
    writeLevelData(changedOnDisk, levelPath);

    REQUIRE(levelUi.takesTheDisk(levelData, levelPath));
    REQUIRE_FALSE(levelUi.unsavedSince(readLevelData(levelPath), levelPath));
    REQUIRE(levelUi.unsavedSince(levelData, levelPath));
}

TEST_CASE("A level never looked at takes the disk only where it differs", "[UnsavedSections]")
{
    LevelUi levelUi;
    ACopyOfLevelOne copy;
    const std::string &levelPath = copy.path;
    LevelData levelData = readLevelData(levelPath);
    REQUIRE_FALSE(levelUi.takesTheDisk(levelData, levelPath));

    LevelUi another;
    LevelData changedOnDisk = levelData;
    changedOnDisk.playerFeet.x += 16.0f;
    writeLevelData(changedOnDisk, levelPath);

    REQUIRE(another.takesTheDisk(levelData, levelPath));
}

TEST_CASE(
    "A clean section says when a reload changed it, and takes the change",
    "[UnsavedSections]")
{
    CameraUi cameraUi;
    GameData gameData;
    REQUIRE_FALSE(cameraUi.unsavedSince(gameData));

    GameData onDisk = gameData;
    REQUIRE_FALSE(cameraUi.reloaded(gameData, onDisk));

    onDisk.cameraData.zoom += 1.0f;
    REQUIRE(cameraUi.reloaded(gameData, onDisk));
    REQUIRE(gameData.cameraData.zoom == onDisk.cameraData.zoom);

    REQUIRE_FALSE(cameraUi.reloaded(gameData, onDisk));
}

TEST_CASE(
    "A section with unsaved edits keeps them through a reload and says nothing changed",
    "[UnsavedSections]")
{
    CameraUi cameraUi;
    GameData gameData;
    REQUIRE_FALSE(cameraUi.unsavedSince(gameData));
    gameData.cameraData.zoom += 1.0f;

    GameData onDisk;
    onDisk.cameraData.zoom += 2.0f;
    REQUIRE_FALSE(cameraUi.reloaded(gameData, onDisk));

    REQUIRE(gameData.cameraData.zoom == 1.0f + GameData().cameraData.zoom);
}

TEST_CASE(
    "The first-level gate reads the disk once per path, and again when the path changes",
    "[UnsavedSections]")
{
    LevelsUi levelsUi;
    ACopyOfLevelOne copy;
    LevelsData levels;
    levels.first.path = copy.path;
    REQUIRE_FALSE(levelsUi.cannotSaveBecause(levels).has_value());

    std::filesystem::remove(copy.path);
    REQUIRE_FALSE(levelsUi.cannotSaveBecause(levels).has_value());

    levels.first.path = copy.path + ".gone";
    std::optional<std::string> why = levelsUi.cannotSaveBecause(levels);
    REQUIRE(why.has_value());
    REQUIRE_THAT(*why, Catch::Matchers::ContainsSubstring("cannot be read"));
}
