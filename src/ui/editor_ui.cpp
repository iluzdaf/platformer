#include <array>
#include <cfloat>
#include <cstddef>
#include <string>
#include <imgui.h>
#include <string_view>
#include "ui/editor_ui.hpp"
#include "ui/mouse_on_the_map.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "player/player.hpp"
#include "tile_map/tile_map.hpp"
#include "ui/editor_section.hpp"
#include "ui/imgui_manager.hpp"
#include "ui/debug_aabb_overlay.hpp"
#include "ui/save_controls.hpp"

namespace
{

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

    std::array<bool, EditorSections.size()> unsaved{};
    std::string unsavedNames;
    for (std::size_t at = 0; at < EditorSections.size(); ++at)
    {
        const auto &[listed, name] = EditorSections[at];
        unsaved[at] = unsavedIn(listed, subject);
        if (unsaved[at])
            unsavedNames += (unsavedNames.empty() ? "" : ", ") + std::string(name);
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage) the names are string literals
    if (ImGui::BeginCombo("##section", nameOf(section).data()))
    {
        for (std::size_t at = 0; at < EditorSections.size(); ++at)
        {
            const auto &[listed, name] = EditorSections[at];
            if (unsaved[at])
                ImGui::PushStyleColor(ImGuiCol_Text, UnsavedColour);

            // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage) a string literal again
            if (ImGui::Selectable(name.data(), listed == section))
                section = listed;

            if (unsaved[at])
                ImGui::PopStyleColor();
        }

        ImGui::EndCombo();
    }

    if (!unsavedNames.empty())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, UnsavedColour);
        ImGui::TextWrapped("unsaved in %s", unsavedNames.c_str());
        ImGui::PopStyleColor();
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
        playerUi.draw(subject.gameData, commands);
        break;

    case EditorSection::NpcTypes:
        npcTypesUi.draw(subject.gameData);
        break;

    case EditorSection::Levels:
        levelsUi.draw(
            subject.levels, subject.level, commands, levelUi.hasUnsavedChanges(subject.level));
        break;

    case EditorSection::Level:
        levelUi.draw(
            subject.level,
            subject.npcs,
            subject.playerMotionState,
            subject.playerFeet,
            subject.playerState,
            subject.tileSet,
            subject.gameData,
            armed,
            commands);
        break;

    case EditorSection::TilePalettes:
        tilePalettesUi.draw(
            subject.gameData.tilePalettes,
            subject.tileSet,
            subject.level.getTileMap().getTileSize(),
            armed);
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
    playerUi.drawOverlay(imGuiManager, camera, player);
}

void EditorUi::update(
    float deltaTime,
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    Level &level)
{
    playerUi.update(deltaTime);
    MouseOnTheMap mouse{
        imGuiManager.getIO().WantCaptureMouse,
        imGuiManager.screenToWorld(
            ImGui::GetMousePos(), camera.getZoom(), camera.getTopLeftPosition()),
        ImGui::IsMouseDown(ImGuiMouseButton_Left),
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)};

    levelUi.update(mouse, level, armed, commands);
}

bool EditorUi::unsavedIn(EditorSection listed, const EditorSubject &subject) const
{
    switch (listed)
    {
    case EditorSection::Game:
        return gameSettingsUi.hasUnsavedChanges(subject.gameData);

    case EditorSection::Camera:
        return cameraUi.hasUnsavedChanges(subject.gameData);

    case EditorSection::Player:
        return playerUi.hasUnsavedChanges(subject.gameData);

    case EditorSection::Levels:
        return levelsUi.hasUnsavedChanges(subject.levels);

    case EditorSection::Level:
        return levelUi.hasUnsavedChanges(subject.level);

    case EditorSection::NpcTypes:
        return npcTypesUi.hasUnsavedChanges(subject.gameData);

    case EditorSection::TilePalettes:
        return tilePalettesUi.hasUnsavedChanges(subject.gameData.tilePalettes);

    case EditorSection::Playback:
        break;
    }

    return false;
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
