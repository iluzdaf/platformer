#include "ui/game_ui.hpp"
#include "game/level_data.hpp"
#include <string>
#include "rendering/texture_cache.hpp"
#include "assets/sheet_data.hpp"
#include "physics/aabb.hpp"
#include "physics/physics_body.hpp"
#include "ui/editor_commands.hpp"
#include "ui/editor_ui.hpp"
#include "ui/score_ui.hpp"
#include "game/score_icon_data.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "player/player.hpp"
#include "window/window.hpp"

GameUi::GameUi(const Window &window, int width, int height)
    : imGuiManager(window.getHandle(), width, height)
{
}

void GameUi::draw(const GameUiSubject &subject)
{
    imGuiManager.newFrame();

    const ScoreIconData &scoreIcon = subject.gameData.settings.scoreIcon;
    drawScore(
        imGuiManager, subject.score, subject.textures.get(scoreIcon.sheet.texture), scoreIcon);

    editorUi.draw(
        imGuiManager,
        EditorSubject{
            subject.gameData,
            subject.level,
            subject.levelData,
            subject.levelPath,
            subject.textures,
            subject.levels,
            subject.player.getMotion().getState(),
            subject.player.getPhysicsBody().getAABB().bottomCenter(),
            subject.player.getState(),
            subject.camera,
            subject.paused},
        subject.showEditors);

    if (subject.showEditors)
        editorUi.drawOverlays(imGuiManager, subject.camera, subject.level, subject.player);

    imGuiManager.render();
}

void GameUi::update(
    float deltaTime,
    const Level &level,
    const LevelData &levelData,
    const std::string &levelPath,
    const Camera2D &camera)
{
    editorUi.update(deltaTime, imGuiManager, camera, level, levelData, levelPath);
}

void GameUi::resize(int width, int height)
{
    imGuiManager.resize(width, height);
}

EditorUi &GameUi::editor()
{
    return editorUi;
}

EditorCommands &GameUi::commands()
{
    return editorUi.commands;
}
