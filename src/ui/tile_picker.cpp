#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/brush.hpp"
#include "ui/tile_picker.hpp"
#include "rendering/texture2d.hpp"

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

    bool drawPlayerStartCell()
    {
        ImVec2 padding = ImGui::GetStyle().FramePadding;

        return ImGui::Button(
            "spawn",
            ImVec2(TilePickerCellSize + padding.x * 2.0f, TilePickerCellSize + padding.y * 2.0f));
    }
}

std::optional<Brush> drawTilePicker(
    const Texture2D &tileSet,
    int tileSize,
    const std::vector<Brush> &brushes,
    std::optional<Brush> armed)
{
    std::optional<Brush> picked = armed;
    const ImGuiStyle &style = ImGui::GetStyle();
    float rightEdge = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;

    for (std::size_t at = 0; at < brushes.size(); ++at)
    {
        const Brush &brush = brushes[at];
        bool isArmed = armed && *armed == brush;

        ImGui::PushID(static_cast<int>(at));

        if (isArmed)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, TilePickerArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, TilePickerArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, TilePickerArmedColour);
        }

        bool clicked = false;
        switch (brush.kind)
        {
        case Brush::Kind::Tile:
            clicked = drawTileCell(tileSet, tileSize, brush.tileIndex);
            break;

        case Brush::Kind::PlayerStart:
            clicked = drawPlayerStartCell();
            break;
        }

        if (clicked)
            picked = isArmed ? std::nullopt : std::optional<Brush>(brush);

        float nextRightEdge =
            ImGui::GetItemRectMax().x + style.ItemSpacing.x + ImGui::GetItemRectSize().x;

        if (isArmed)
            ImGui::PopStyleColor(3);

        if (at + 1 < brushes.size() && nextRightEdge < rightEdge)
            ImGui::SameLine();

        ImGui::PopID();
    }

    return picked;
}
