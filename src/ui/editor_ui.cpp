#include <cfloat>
#include <string_view>
#include <utility>
#include <imgui.h>
#include "ui/editor_ui.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "player/player.hpp"
#include "tile_map/tile_map.hpp"
#include "ui/editor_section.hpp"
#include "ui/imgui_manager.hpp"
#include "ui/debug_aabb_overlay.hpp"

namespace
{
    constexpr float InspectorWidth = 260.0f;

    std::string_view nameOf(EditorSection section)
    {
        for (const auto &[listed, name] : EditorSections)
            if (listed == section)
                return name;

        return {};
    }
}

void EditorUi::draw(
    const ImGuiManager &imGuiManager,
    const EditorSubject &subject,
    bool showEditors)
{
    if (!showEditors)
        return;

    ImVec2 displaySize = imGuiManager.getUiDimensions();
    ImGui::SetNextWindowPos(ImVec2(displaySize.x - InspectorWidth, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(InspectorWidth, displaySize.y), ImGuiCond_Always);

    if (!ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::End();
        return;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##section", nameOf(section).data()))
    {
        for (const auto &[listed, name] : EditorSections)
            if (ImGui::Selectable(name.data(), listed == section))
                section = listed;

        ImGui::EndCombo();
    }

    ImGui::Separator();

    switch (section)
    {
    case EditorSection::Playback:
        playbackUi.draw(subject.paused, commands);
        break;

    case EditorSection::Game:
        gameSettingsUi.draw(subject.gameData, commands);
        break;

    case EditorSection::Camera:
        cameraUi.draw(subject.gameData, subject.camera, commands);
        break;

    case EditorSection::Player:
        playerUi.draw(
            subject.gameData,
            subject.playerMotionState,
            subject.playerPosition,
            subject.playerState,
            commands);
        break;

    case EditorSection::NpcTypes:
        npcTypesUi.draw(subject.gameData);
        break;

    case EditorSection::Levels:
        levelsUi.draw(
            subject.levels, subject.level, commands, levelUi.hasUnsavedChanges(subject.level));
        break;

    case EditorSection::Level:
        levelUi.draw(section, subject.level, subject.tileSet, subject.gameData, brush, commands);
        break;

    case EditorSection::NpcsInLevel:
        npcsUi.draw(subject.level, subject.npcs);
        break;

    case EditorSection::Navigation:
        navigationUi.draw(subject.level);
        break;

    case EditorSection::TilePalettes:
        tilePalettesUi.draw(
            subject.gameData.tilePalettes,
            subject.tileSet,
            subject.level.getTileMap().getTileSize(),
            brush);
        break;
    }

    ImGui::End();
}

void EditorUi::drawOverlays(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level,
    const Player &player)
{
    levelUi.drawOverlay(imGuiManager, camera, level);
    navigationUi.drawOverlay(imGuiManager, camera, level);
    if (playerUi.drawsPlayerCollider())
        drawPlayerCollider(imGuiManager, camera, player);
    if (playerUi.drawsPlayerCollisions())
        drawPlayerCollisions(player, fadingAABBs);
    if (levelUi.drawsTileColliders())
        drawTileColliders(imGuiManager, camera, level);
    if (levelUi.drawsLevelBounds())
        drawLevelBounds(imGuiManager, camera, level);
    if (levelUi.drawsPlayerStart())
        drawPlayerStart(imGuiManager, camera, level);
    if (playerUi.drawsContactProbes())
        drawContactProbes(imGuiManager, camera, player, fadingAABBs);
    drawFadingAABBs(imGuiManager, camera, fadingAABBs);
}

void EditorUi::update(
    float deltaTime,
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    Level &level)
{
    fadingAABBs.update(deltaTime);
    levelUi.update(imGuiManager, camera, level, brush);
}

void EditorUi::valuesReplaced()
{
    gameSettingsUi.valuesReplaced();
    cameraUi.valuesReplaced();
    playerUi.valuesReplaced();
    npcTypesUi.valuesReplaced();
    tilePalettesUi.valuesReplaced();
    levelUi.valuesReplaced();
    levelsUi.valuesReplaced();
}
