#include <utility>
#include <imgui.h>
#include "rendering/ui/editor_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"

namespace
{
    constexpr float SectionsWidth = 140.0f;
    constexpr float InspectorWidth = 260.0f;
}

void EditorUi::draw(const ImGuiManager &imGuiManager, bool showEditors)
{
    if (!showEditors)
        return;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(SectionsWidth, imGuiManager.getUiDimensions().y));
    ImGui::Begin("Sections", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    for (const auto &[listed, name] : EditorSections)
        if (ImGui::Selectable(name.data(), listed == section))
            section = listed;

    ImGui::End();
}

EditorSection EditorUi::getSection() const
{
    return section;
}

void EditorUi::beginInspector(const ImGuiManager &imGuiManager, const char *title)
{
    ImVec2 displaySize = imGuiManager.getUiDimensions();
    ImGui::SetNextWindowPos(ImVec2(displaySize.x - InspectorWidth, 0));
    ImGui::SetNextWindowSize(ImVec2(InspectorWidth, displaySize.y));
    ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoResize);
}

void EditorUi::endInspector()
{
    ImGui::End();
}
