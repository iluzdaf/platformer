#include <cfloat>
#include <optional>
#include <string>
#include <vector>
#include <imgui.h>
#include "ui/tile_palettes_ui.hpp"
#include "ui/save_controls.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/armed.hpp"
#include "ui/tile_picker.hpp"
#include "game/game_data.hpp"
#include "tile_map/tile_palette.hpp"

void TilePalettesUi::draw(
    TilePalettes &tilePalettes,
    const Texture2D &tileSet,
    int tileSize,
    std::optional<Armed> &armed)
{
    drawSaveControls(saveable, "palettes", tilePalettes, saveTilePalettes);
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

    std::optional<int> showing = paintedTile(armed);
    if (showing && !palette.contains(*showing))
        showing.reset();

    std::optional<int> picked = drawTilePicker(tileSet, tileSize, tileIndices, showing);
    if (picked != showing)
        armed = picked ? std::optional<Armed>(PaintTile{*picked}) : std::nullopt;

    ImGui::Separator();
    if (!picked)
    {
        ImGui::TextDisabled("pick a tile");
        return;
    }

    ImGui::Text("tile %d", *picked);
    inspector::drawFields(palette.at(*picked));
}

bool TilePalettesUi::hasUnsavedChanges(const TilePalettes &tilePalettes) const
{
    return saveable.unsaved("palettes", asJson(tilePalettes));
}

void TilePalettesUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
