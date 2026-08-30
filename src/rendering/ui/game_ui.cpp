#include "rendering/ui/game_ui.hpp"
#include "rendering/ui/editor_commands.hpp"
#include "rendering/ui/editor_ui.hpp"
#include "game/level.hpp"
#include "player/player.hpp"
#include "window/window.hpp"

GameUi::GameUi(Window &window, int width, int height)
    : imGuiManager(window.getHandle(), width, height)
{
}

void GameUi::draw(const GameUiSubject &subject)
{
    imGuiManager.newFrame();

    scoreUi.draw(imGuiManager, subject.scoringSystem, subject.tileSet);

    editorUi.draw(
        imGuiManager,
        EditorSubject{
            subject.gameData,
            subject.level,
            subject.npcs,
            subject.tileSet,
            subject.levels,
            subject.player.getMotion().getState(),
            subject.player.getPosition(),
            subject.player.getState(),
            subject.camera,
            subject.paused},
        subject.showEditors);

    if (subject.showEditors)
        editorUi.drawOverlays(imGuiManager, subject.camera, subject.level, subject.player);

    imGuiManager.render();
}

void GameUi::update(float deltaTime, Level &level, const Camera2D &camera)
{
    editorUi.update(deltaTime, imGuiManager, camera, level);
}

void GameUi::resize(int width, int height)
{
    imGuiManager.resize(width, height);
}

void GameUi::valuesReplaced()
{
    editorUi.valuesReplaced();
}

EditorCommands &GameUi::commands()
{
    return editorUi.commands;
}
