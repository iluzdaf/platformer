#include <cstdint>
#include <optional>
#include <format>
#include <string>
#include "ui/score_ui.hpp"
#include "rendering/texture2d.hpp"
#include "ui/imgui_manager.hpp"
#include "game/scoring_system.hpp"
#include "tile_map/tile_palette.hpp"
#include "tile_map/tile_set.hpp"
#include "tile_map/tile_index.hpp"

void ScoreUi::draw(
    const ImGuiManager &,
    const ScoringSystem &scoringSystem,
    const Texture2D &tileSet,
    const TilePalette &palette)
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
    if (palette.scoreTile)
    {
        auto [uvStart, uvEnd] =
            tileSet.getUVRange(palette.scoreTile->value, palette.tileSet.tileSize, false);
        ImGui::Image(
            (ImTextureID)(intptr_t)tileSet.getTextureID(),
            ImVec2(32, 32),
            ImVec2(uvStart.x, uvStart.y),
            ImVec2(uvEnd.x, uvEnd.y));

        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
    }

    std::string label = std::format("x{}", scoringSystem.getScore());
    ImGui::TextUnformatted(label.c_str());
    ImGui::End();
}