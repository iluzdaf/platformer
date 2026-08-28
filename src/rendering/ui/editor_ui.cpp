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

bool EditorUi::begin(const ImGuiManager &imGuiManager, bool showEditors)
{
    if (!showEditors)
        return false;

    ImVec2 displaySize = imGuiManager.getUiDimensions();
    ImGui::SetNextWindowPos(ImVec2(displaySize.x - InspectorWidth, 0));
    ImGui::SetNextWindowSize(ImVec2(InspectorWidth, displaySize.y));
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##section", nameOf(section).data()))
    {
        for (const auto &[listed, name] : EditorSections)
            if (ImGui::Selectable(name.data(), listed == section))
                section = listed;

        ImGui::EndCombo();
    }

    ImGui::Separator();

    return true;
}

void EditorUi::end()
{
    ImGui::End();
}

EditorSection EditorUi::getSection() const
{
    return section;
}
