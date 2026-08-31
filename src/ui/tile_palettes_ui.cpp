#include <cfloat>
#include <optional>
#include <string>
#include <vector>
#include <imgui.h>
#include "ui/tile_palettes_ui.hpp"
#include "ui/save_controls.hpp"
#include "ui/data_inspector.hpp"
#include "ui/brush.hpp"
#include "ui/brush_picker.hpp"
#include "game/game_data.hpp"
#include "tile_map/tile_palette.hpp"

void TilePalettesUi::draw(
    TilePalettes &tilePalettes,
    const Texture2D &tileSet,
    int tileSize,
    std::optional<Brush> &brush)
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

    std::optional<int> showing;
    if (brush && brush->kind == Brush::Kind::Tile && palette.contains(brush->tileIndex))
        showing = brush->tileIndex;

    std::vector<Brush> brushes;
    for (int tileIndex : tileIndices)
        brushes.push_back(Brush{Brush::Kind::Tile, tileIndex});

    std::optional<Brush> armed =
        showing ? std::optional<Brush>(Brush{Brush::Kind::Tile, *showing}) : std::nullopt;
    brush = drawBrushPicker(tileSet, tileSize, brushes, armed);
    std::optional<int> picked = brush && brush->kind == Brush::Kind::Tile
                                    ? std::optional<int>(brush->tileIndex)
                                    : std::nullopt;

    ImGui::Separator();
    if (!picked)
    {
        ImGui::TextDisabled("pick a tile");
        return;
    }

    ImGui::Text("tile %d", *picked);
    inspector::drawFields(palette.at(*picked));
}

void TilePalettesUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
