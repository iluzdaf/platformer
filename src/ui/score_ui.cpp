#include <cstdint>
#include <format>
#include <string>
#include "ui/score_ui.hpp"
#include "rendering/texture2d.hpp"
#include "ui/imgui_manager.hpp"
#include "game/score.hpp"
#include "game/score_icon_data.hpp"

void drawScore(
    const ImGuiManager &,
    const Score &score,
    const Texture2D &icon,
    const ScoreIconData &scoreIcon)
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::Begin(
        "Score",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground);
    ImGui::SetWindowFontScale(2.0f);

    auto [uvStart, uvEnd] = frameUvRangeIn(
        static_cast<int>(icon.getWidth()),
        static_cast<int>(icon.getHeight()),
        scoreIcon.frame,
        scoreIcon.sheet.cellSize.x,
        scoreIcon.sheet.cellSize.y,
        false);

    ImGui::Image(
        (ImTextureID)(intptr_t)icon.getTextureID(),
        ImVec2(32, 32),
        ImVec2(uvStart.x, uvStart.y),
        ImVec2(uvEnd.x, uvEnd.y));

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
    ImGui::TextUnformatted(std::format("x{}", score.total()).c_str());
    ImGui::End();
}
