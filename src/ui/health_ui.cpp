#include <imgui.h>
#include "ui/health_ui.hpp"
#include "actor/health.hpp"

namespace
{
    constexpr float MarkSize = 14.0f;
    constexpr float MarkGap = 4.0f;
    constexpr ImU32 FullMark = IM_COL32(220, 60, 60, 255);
    constexpr ImU32 EmptyMark = IM_COL32(220, 60, 60, 90);
}

void drawHealth(const ImGuiManager &, const Health &health)
{
    ImGui::SetNextWindowPos(ImVec2(10, 52), ImGuiCond_Always);
    ImGui::Begin(
        "Health",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground);

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 at = ImGui::GetCursorScreenPos();
    for (int mark = 0; mark < health.maximum(); ++mark)
    {
        ImVec2 topLeft(at.x + static_cast<float>(mark) * (MarkSize + MarkGap), at.y);
        ImVec2 bottomRight(topLeft.x + MarkSize, topLeft.y + MarkSize);
        drawList->AddRectFilled(
            topLeft, bottomRight, mark < health.points() ? FullMark : EmptyMark);
    }

    ImGui::Dummy(
        ImVec2(static_cast<float>(health.maximum()) * (MarkSize + MarkGap) - MarkGap, MarkSize));
    ImGui::End();
}
