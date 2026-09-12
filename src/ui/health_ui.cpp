#include <cstdint>
#include <imgui.h>
#include "ui/health_ui.hpp"
#include "combat/health.hpp"
#include "game/health_icon_data.hpp"
#include "rendering/texture2d.hpp"

namespace
{
    constexpr float MarkSize = 24.0f;
    constexpr float MarkGap = 2.0f;
}

void drawHealth(
    const ImGuiManager &,
    const Health &health,
    const Texture2D &icon,
    const HealthIconData &healthIcon)
{
    ImGui::SetNextWindowPos(ImVec2(10, 52), ImGuiCond_Always);
    ImGui::Begin(
        "Health",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground);

    for (int mark = 0; mark < health.maximum(); ++mark)
    {
        if (mark > 0)
            ImGui::SameLine(0.0f, MarkGap);

        auto [uvStart, uvEnd] = frameUvRangeIn(
            static_cast<int>(icon.getWidth()),
            static_cast<int>(icon.getHeight()),
            mark < health.points() ? healthIcon.full : healthIcon.spent,
            healthIcon.sheet.cellSize.x,
            healthIcon.sheet.cellSize.y,
            false);

        ImGui::Image(
            (ImTextureID)(intptr_t)icon.getTextureID(),
            ImVec2(MarkSize, MarkSize),
            ImVec2(uvStart.x, uvStart.y),
            ImVec2(uvEnd.x, uvEnd.y));
    }

    ImGui::End();
}
