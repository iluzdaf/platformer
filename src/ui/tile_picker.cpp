#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/tile_picker.hpp"
#include "rendering/texture2d.hpp"

std::optional<int> drawTilePicker(
    const Texture2D &tileSet,
    int tileSize,
    const std::vector<int> &tileIndices,
    std::optional<int> selected)
{
    ImTextureID imguiTextureID = (ImTextureID)(intptr_t)tileSet.getTextureID();
    std::optional<int> picked = selected;
    const ImGuiStyle &style = ImGui::GetStyle();
    float rightEdge = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;

    for (std::size_t at = 0; at < tileIndices.size(); ++at)
    {
        int tileIndex = tileIndices[at];
        ImGui::PushID(tileIndex);
        auto [uvStart, uvEnd] = tileSet.getUVRange(tileIndex, tileSize, false);
        bool isSelected = tileIndex == selected;

        if (isSelected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, TilePickerArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, TilePickerArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, TilePickerArmedColour);
        }

        ImVec2 tilePosition = ImGui::GetCursorScreenPos();
        if (ImGui::ImageButton(
                "##tile",
                imguiTextureID,
                ImVec2(TilePickerCellSize, TilePickerCellSize),
                ImVec2(uvStart.x, uvStart.y),
                ImVec2(uvEnd.x, uvEnd.y)))
            picked = isSelected ? std::nullopt : std::optional<int>(tileIndex);

        float nextRightEdge =
            ImGui::GetItemRectMax().x + style.ItemSpacing.x + ImGui::GetItemRectSize().x;

        ImGui::GetWindowDrawList()->AddText(
            tilePosition, IM_COL32(255, 255, 255, 255), std::to_string(tileIndex).c_str());

        if (isSelected)
            ImGui::PopStyleColor(3);

        if (at + 1 < tileIndices.size() && nextRightEdge < rightEdge)
            ImGui::SameLine();

        ImGui::PopID();
    }

    return picked;
}
