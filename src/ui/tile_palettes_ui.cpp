#include <cfloat>
#include <optional>
#include <string>
#include <imgui.h>
#include "ui/tile_palettes_ui.hpp"
#include "ui/save_controls.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/armed.hpp"
#include "ui/tile_picker.hpp"
#include "rendering/tile_set_textures.hpp"
#include "tile_map/tile_data.hpp"
#include "game/game_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/texture_cache.hpp"
#include "ui/editor_commands.hpp"
#include "tile_map/tile_set.hpp"

void TilePalettesUi::draw(
    TilePalettes &tilePalettes,
    const TextureCache &textures,
    EditorCommands &commands,
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

    ImGui::Separator();
    inspector::drawFields(palette.tileSet);
    ImGui::Separator();

    const Texture2D *tileSet = textures.find(palette.tileSet.texture);
    if (!tileSet)
    {
        if (!palette.tileSet.texture.empty() && palette.tileSet.texture != askedToWarm)
        {
            askedToWarm = palette.tileSet.texture;
            commands.onWarmTexture(palette.tileSet.texture);
        }

        ImGui::TextDisabled("no texture at %s", palette.tileSet.texture.c_str());
        return;
    }

    int cells = tilesInSheet(
        static_cast<int>(tileSet->getWidth()),
        static_cast<int>(tileSet->getHeight()),
        palette.tileSet.tileSize);
    if (cells <= 0)
    {
        ImGui::TextDisabled("no whole tiles in this tile set");
        return;
    }

    std::optional<int> showing = paintedTile(armed);
    if (showing && *showing >= cells)
        showing.reset();

    std::optional<int> picked = drawTilePicker(*tileSet, palette.tileSet, showing);
    if (picked != showing)
        armed = picked ? std::optional<Armed>(PaintTile{*picked}) : std::nullopt;

    ImGui::Separator();
    if (!picked)
    {
        ImGui::TextDisabled("pick a tile");
        return;
    }

    ImGui::Text("tile %d", *picked);

    auto known = palette.tiles.find(*picked);
    if (known != palette.tiles.end())
    {
        inspector::drawFields(known->second);
        return;
    }

    TileData nothingSaid;
    if (inspector::drawFields(nothingSaid))
        palette.tiles.insert({*picked, nothingSaid});
}

bool TilePalettesUi::hasUnsavedChanges(const TilePalettes &tilePalettes) const
{
    return saveable.unsaved("palettes", asJson(tilePalettes));
}

void TilePalettesUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
