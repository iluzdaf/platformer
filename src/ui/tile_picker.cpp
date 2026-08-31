#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/armed.hpp"
#include "ui/tile_picker.hpp"
#include "rendering/texture2d.hpp"
#include "tile_map/tile_map.hpp"

namespace
{
    bool drawTileCell(const Texture2D &tileSet, int tileSize, int tileIndex)
    {
        auto [uvStart, uvEnd] = tileSet.getUVRange(tileIndex, tileSize, false);
        ImVec2 cellPosition = ImGui::GetCursorScreenPos();

        bool clicked = ImGui::ImageButton(
            "##tile",
            (ImTextureID)(intptr_t)tileSet.getTextureID(),
            ImVec2(TilePickerCellSize, TilePickerCellSize),
            ImVec2(uvStart.x, uvStart.y),
            ImVec2(uvEnd.x, uvEnd.y));

        ImGui::GetWindowDrawList()->AddText(
            cellPosition, IM_COL32(255, 255, 255, 255), std::to_string(tileIndex).c_str());

        return clicked;
    }
}

std::vector<int> tilesToPickFrom(const TileMap &tileMap)
{
    std::vector<int> tileIndices;
    for (const auto &[tileIndex, tile] : tileMap.getTiles())
        tileIndices.push_back(tileIndex);

    std::sort(tileIndices.begin(), tileIndices.end());

    return tileIndices;
}

std::optional<int> drawTilePicker(
    const Texture2D &tileSet,
    int tileSize,
    const std::vector<int> &tileIndices,
    std::optional<int> armed)
{
    std::optional<int> picked = armed;
    const ImGuiStyle &style = ImGui::GetStyle();
    float rightEdge = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;

    for (std::size_t at = 0; at < tileIndices.size(); ++at)
    {
        int tileIndex = tileIndices[at];
        bool isArmed = armed == tileIndex;

        ImGui::PushID(static_cast<int>(at));

        if (isArmed)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ArmedColour);
        }

        if (drawTileCell(tileSet, tileSize, tileIndex))
            picked = isArmed ? std::nullopt : std::optional<int>(tileIndex);

        float nextRightEdge =
            ImGui::GetItemRectMax().x + style.ItemSpacing.x + ImGui::GetItemRectSize().x;

        if (isArmed)
            ImGui::PopStyleColor(3);

        if (at + 1 < tileIndices.size() && nextRightEdge < rightEdge)
            ImGui::SameLine();

        ImGui::PopID();
    }

    return picked;
}
