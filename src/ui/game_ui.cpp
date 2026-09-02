#include "ui/game_ui.hpp"
#include "rendering/texture_cache.hpp"
#include "tile_map/tile_map.hpp"
#include "assets/sheet.hpp"
#include "physics/aabb.hpp"
#include "physics/physics_body.hpp"
#include "ui/editor_commands.hpp"
#include "ui/editor_ui.hpp"
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

    const Sheet &tileSet = subject.level.getTileMap().getTileSet();
    scoreUi.draw(
        imGuiManager,
        subject.scoringSystem,
        subject.textures.get(tileSet.texture),
        tileSet.cellSize.x);

    editorUi.draw(
        imGuiManager,
        EditorSubject{
            subject.gameData,
            subject.level,
            subject.npcs,
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
