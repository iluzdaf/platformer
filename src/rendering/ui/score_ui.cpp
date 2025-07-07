#include <format>
#include "rendering/ui/score_ui.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "game/scoring_system.hpp"

void ScoreUi::draw(
    const ImGuiManager &,
    const ScoringSystem &scoringSystem,
    const Texture2D &tileSet)
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::Begin("Score",
                 nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoNav |
                     ImGuiWindowFlags_NoBackground);
    ImGui::SetWindowFontScale(2.0f);
    ImTextureID imguiTextureID = (ImTextureID)(intptr_t)tileSet.getTextureID();
    auto [uvStart, uvEnd] = tileSet.getUVRange(42, 16, false);
    ImGui::Image(imguiTextureID, ImVec2(32, 32), ImVec2(uvStart.x, uvStart.y), ImVec2(uvEnd.x, uvEnd.y));
    std::string label = std::format("x{}", scoringSystem.getScore());
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
    ImGui::TextUnformatted(label.c_str());
    ImGui::End();
}