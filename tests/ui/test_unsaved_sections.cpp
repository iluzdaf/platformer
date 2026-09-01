#include <catch2/catch_test_macros.hpp>
#include "cameras/camera2d.hpp"
#include "cameras/camera2d_data.hpp"
#include "game/game_data.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "game/level.hpp"
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

    REQUIRE_FALSE(cameraUi.hasUnsavedChanges(gameData));
}

TEST_CASE("A section reports unsaved once its data changes", "[UnsavedSections]")
{
    HeadlessImGui gui;
    CameraUi cameraUi;
    GameData gameData;

    drawCameraOnce(gui, cameraUi, gameData);
    gameData.cameraData.zoom += 1.0f;

    REQUIRE(cameraUi.hasUnsavedChanges(gameData));
}

TEST_CASE("A section that has never been opened reports nothing", "[UnsavedSections]")
{
    CameraUi cameraUi;
    GameData gameData;
    gameData.cameraData.zoom += 1.0f;

    REQUIRE_FALSE(cameraUi.hasUnsavedChanges(gameData));
}

TEST_CASE("The levels section has nothing unsaved when it is first drawn", "[UnsavedSections]")
{
    HeadlessImGui gui;
    LevelsUi levelsUi;
    Levels levels(assetPath("levels.json"));
    GameData gameData = loadGameData();
    Level level(
        assetPath(levels.getFirst()), gameData.tilePalettes, gameData.playerData, gameData.npcData);
    EditorCommands commands;

    gui.frame([&] { levelsUi.draw(levels, level, commands, false); });

    REQUIRE_FALSE(levelsUi.hasUnsavedChanges(levels));
}

TEST_CASE("The levels section reports unsaved once the first level changes", "[UnsavedSections]")
{
    HeadlessImGui gui;
    LevelsUi levelsUi;
    Levels levels(assetPath("levels.json"));
    GameData gameData = loadGameData();
    Level level(
        assetPath(levels.getFirst()), gameData.tilePalettes, gameData.playerData, gameData.npcData);
    EditorCommands commands;

    gui.frame([&] { levelsUi.draw(levels, level, commands, false); });
    levels.setFirst("levels/level3.json");

    REQUIRE(levelsUi.hasUnsavedChanges(levels));
}

#ifndef SKIP_OPENGL_TESTS
#include <memory>
#include <optional>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_motion_state.hpp"
#include "actor/actor_state.hpp"
#include "npc/npc.hpp"
#include "rendering/texture2d.hpp"
#include "ui/armed.hpp"
#include "ui/level_ui.hpp"

TEST_CASE("A level edited with the inspector shut still reports unsaved", "[UnsavedSections]")
{
    HeadlessImGui gui;
    LevelUi levelUi;
    GameData gameData = loadGameData();
    Level level(
        assetPath("levels/level1.json"),
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData);
    Texture2D tileSet(assetPath("textures/tile_set.png"));

    std::vector<std::unique_ptr<Npc>> npcs;
    ActorMotionState motion;
    ActorState playerState;
    std::optional<Armed> armed;
    EditorCommands commands;

    auto drawOnce = [&]
    {
        gui.frame(
            [&]
            {
                levelUi.draw(
                    level,
                    npcs,
                    motion,
                    level.getTileMap().feetOnTile(glm::ivec2(1, 1)),
                    playerState,
                    tileSet,
                    gameData,
                    armed,
                    commands);
            });
    };

    drawOnce();
    REQUIRE_FALSE(levelUi.hasUnsavedChanges(level));

    level.setNextLevel("levels/level3.json");
    drawOnce();

    REQUIRE(levelUi.hasUnsavedChanges(level));
}
#endif
