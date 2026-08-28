#include <cfloat>
#include <string_view>
#include <utility>
#include <imgui.h>
#include "rendering/ui/editor_ui.hpp"
#include "rendering/ui/editor_section.hpp"
#include "rendering/ui/imgui_manager.hpp"

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
    case EditorSection::Level:
    case EditorSection::TileMap:
        levelEditorUi.draw(section, subject.level, subject.tileSet, subject.firstLevel, commands);
        break;

    case EditorSection::NpcsInLevel:
        npcsUi.draw(subject.level, subject.npcs);
        break;

    case EditorSection::Navigation:
        navigationUi.draw(subject.level);
        break;

    default:
        gameEditorUi.draw(
            section,
            subject.gameData,
            subject.playerMotionState,
            subject.playerPosition,
            subject.playerState,
            subject.camera,
            subject.paused,
            commands);
        break;
    }

    ImGui::End();
}

void EditorUi::drawOverlays(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level) const
{
    levelEditorUi.drawOverlay(imGuiManager, camera, level);
    navigationUi.drawOverlay(imGuiManager, camera, level);
}

void EditorUi::update(const ImGuiManager &imGuiManager, const Camera2D &camera, Level &level)
{
    levelEditorUi.update(imGuiManager, camera, level);
}

bool EditorUi::drawsPlayerAABBs() const
{
    return gameEditorUi.drawsPlayerAABBs();
}

bool EditorUi::drawsTileMapAABBs() const
{
    return levelEditorUi.drawsTileMapAABBs();
}
