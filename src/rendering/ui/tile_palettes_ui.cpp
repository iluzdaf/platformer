#include <cfloat>
#include <string>
#include <vector>
#include <imgui.h>
#include "rendering/ui/tile_palettes_ui.hpp"
#include "rendering/ui/data_inspector.hpp"
#include "rendering/ui/tile_picker.hpp"
#include "game/game_data.hpp"
#include "tile_map/tile_palette.hpp"

void TilePalettesUi::draw(
    TilePalettes &tilePalettes,
    const Texture2D &tileSet,
    int tileSize,
    int &selectedTileIndex)
{
    saveable.drawControls("palettes", tilePalettes, saveTilePalettes);
    ImGui::Separator();

    if (tilePalettes.empty())
    {
        ImGui::TextDisabled("no palettes");
        return;
    }

    if (!tilePalettes.contains(selectedPalette))
        selectedPalette = tilePalettes.begin()->first;

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##palette", selectedPalette.c_str()))
    {
        for (const auto &[name, palette] : tilePalettes)
            if (ImGui::Selectable(name.c_str(), name == selectedPalette))
                selectedPalette = name;

        ImGui::EndCombo();
    }

    TilePalette &palette = tilePalettes.at(selectedPalette);
    std::vector<int> tileIndices;
    for (const auto &[tileIndex, tileData] : palette)
        tileIndices.push_back(tileIndex);

    if (tileIndices.empty())
    {
        ImGui::TextDisabled("no tiles in this palette");
        return;
    }

    if (!palette.contains(selectedTileIndex))
        selectedTileIndex = tileIndices.front();

    selectedTileIndex = drawTilePicker(tileSet, tileSize, tileIndices, selectedTileIndex);

    ImGui::Separator();
    ImGui::Text("tile %d", selectedTileIndex);
    inspector::drawFields(palette.at(selectedTileIndex));
}

void TilePalettesUi::forget()
{
    saveable.forget();
}
