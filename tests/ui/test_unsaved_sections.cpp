#include <catch2/catch_test_macros.hpp>
#include "cameras/camera2d.hpp"
#include "cameras/camera2d_data.hpp"
#include "game/game_data.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "game/level.hpp"
#include "game/level_data_file.hpp"
#include "game/levels.hpp"
#include "test_helpers/asset_path.hpp"
#include "ui/camera_ui.hpp"
#include "ui/levels_ui.hpp"
#include "ui/editor_commands.hpp"

namespace
{
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
    Levels levels(assetPath("levels.json"));
    GameData gameData = loadGameData();
    std::string levelPath = assetPath(levels.getFirst());
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
    Levels levels(assetPath("levels.json"));
    GameData gameData = loadGameData();
    std::string levelPath = assetPath(levels.getFirst());
    Level level(
        readLevelData(levelPath),
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);
    EditorCommands commands;

    REQUIRE_FALSE(levelsUi.unsavedSince(levels));
    levels.setFirst("levels/level3.json");

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
    Level level(
        readLevelData(levelPath),
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);

    std::optional<Armed> armed;
    EditorCommands commands;
    MouseOnTheMap still{true, glm::vec2(0.0f), false, false};

    levelUi.update(still, level, levelPath, armed, commands);
    REQUIRE_FALSE(levelUi.unsavedSince(level, levelPath));

    level.setNextLevel("levels/level3.json");
    levelUi.update(still, level, levelPath, armed, commands);

    REQUIRE(levelUi.unsavedSince(level, levelPath));
}

TEST_CASE("A level painted from another section reports unsaved", "[UnsavedSections]")
{
    LevelUi levelUi;
    GameData gameData = loadGameData();
    std::string levelPath = assetPath("levels/level1.json");
    Level level(
        readLevelData(levelPath),
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);

    std::optional<Armed> armed = PaintTile{5};
    EditorCommands commands;
    MouseOnTheMap mouse{false, level.getTileMap().feetOnTile(glm::ivec2(2, 2)), true, false};

    levelUi.update(mouse, level, levelPath, armed, commands);

    REQUIRE(levelUi.unsavedSince(level, levelPath));
}
