#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/armed.hpp"
#include "ui/tile_picker.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/tile_set_fit.hpp"
#include "assets/sheet_data.hpp"

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

std::optional<int> drawTilePicker(
    const Texture2D &sheet,
    const SheetData &tileSet,
    std::optional<int> armed)
{
    int cells = tilesInSheet(
        static_cast<int>(sheet.getWidth()),
        static_cast<int>(sheet.getHeight()),
        tileSet.cellSize.x);

    std::optional<int> picked = armed;
    const ImGuiStyle &style = ImGui::GetStyle();
    float rightEdge = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;

    for (int tileIndex = 0; tileIndex < cells; ++tileIndex)
    {
        bool isArmed = armed == tileIndex;

        ImGui::PushID(tileIndex);

        if (isArmed)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ArmedColour);
        }

        if (drawTileCell(sheet, tileSet.cellSize.x, tileIndex))
            picked = isArmed ? std::nullopt : std::optional<int>(tileIndex);

        float nextRightEdge =
            ImGui::GetItemRectMax().x + style.ItemSpacing.x + ImGui::GetItemRectSize().x;

        if (isArmed)
            ImGui::PopStyleColor(3);

        if (tileIndex + 1 < cells && nextRightEdge < rightEdge)
            ImGui::SameLine();

        ImGui::PopID();
    }

    return picked;
}
