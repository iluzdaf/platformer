#include <cfloat>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/tile_palettes_ui.hpp"
#include "ui/renaming.hpp"
#include "ui/level_rewriting.hpp"
#include "ui/renames.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/armed.hpp"
#include "ui/tile_picker.hpp"
#include "ui/sheet_in_scope.hpp"
#include "rendering/tile_set_fit.hpp"
#include "tile_map/tile_data.hpp"
#include "game/game_data.hpp"
#include "assets/asset_paths.hpp"
#include "game/level_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/texture_cache.hpp"
#include "ui/editor_commands.hpp"
#include "assets/sheet.hpp"

namespace
{
    constexpr float AddWidth = 62.0f;
}

std::string aNameNobodyHasTaken(const TilePalettes &tilePalettes)
{
    for (std::size_t suffix = tilePalettes.size() + 1;; ++suffix)
    {
        std::string name = "palette " + std::to_string(suffix);
        if (!tilePalettes.contains(name))
            return name;
    }
}

void TilePalettesUi::drawChooser(TilePalettes &tilePalettes)
{
    ImGui::SetNextItemWidth(-AddWidth);
    if (ImGui::BeginCombo("##palette", renaming.shownName(selectedPalette).c_str()))
    {
        for (const auto &[name, palette] : tilePalettes)
            if (ImGui::Selectable(renaming.shownName(name).c_str(), name == selectedPalette))
                selectedPalette = name;

        ImGui::EndCombo();
    }

    ImGui::SameLine();
    if (!ImGui::Button("add", ImVec2(-FLT_MIN, 0.0f)))
        return;

    std::string name = aNameNobodyHasTaken(tilePalettes);
    TilePalette made;
    if (tilePalettes.contains(selectedPalette))
        made.tileSet = tilePalettes.at(selectedPalette).tileSet;

    tilePalettes.insert({name, made});
    selectedPalette = name;
}

void TilePalettesUi::drawRename(const TilePalettes &tilePalettes)
{
    if (!renaming.draw(
            "a palette",
            selectedPalette,
            [this, &tilePalettes](const std::string &name)
            { return tilePalettes.contains(name) || renaming.somethingIsBecoming(name); }))
        return;

    lookAheadAtLevels(
        renaming,
        std::string(assets::Levels),
        [](LevelData &levelData, const Renames &renames)
        { return rewriting::paletteIn(levelData.tileMapData, renames); });
}

void TilePalettesUi::draw(
    TilePalettes &tilePalettes,
    const TextureCache &textures,
    EditorCommands &commands,
    std::optional<Armed> &armed)
{
    if (!tilePalettes.empty() && !tilePalettes.contains(selectedPalette))
        selectedPalette = tilePalettes.begin()->first;

    drawChooser(tilePalettes);

    if (tilePalettes.empty())
    {
        ImGui::TextDisabled("no palettes");
        return;
    }

    ImGui::Separator();
    drawRename(tilePalettes);

    TilePalette &palette = tilePalettes.at(selectedPalette);
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
        palette.tileSet.cellSize.x);
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

    ShowingSheet offering(SheetInScope{tileSet, palette.tileSet, *picked});

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
void TilePalettesUi::revert(TilePalettes &tilePalettes)
{
    revertTo(saveable, "palettes", tilePalettes, renaming);
}

void TilePalettesUi::save(TilePalettes &tilePalettes)
{
    renamesTakeEffect(renaming.sinceSaved(), tilePalettes);
    selectedPalette = nameAfterRenames(renaming.sinceSaved(), selectedPalette);

    writeRenamesIntoLevels(
        renaming,
        std::string(assets::Levels),
        [](LevelData &levelData, const Renames &renames)
        { return rewriting::paletteIn(levelData.tileMapData, renames); });

    saveTilePalettes(tilePalettes);
    saveable.saved("palettes", asJson(tilePalettes));
}

bool TilePalettesUi::unsavedSince(const TilePalettes &tilePalettes)
{
    bool values = saveable.unsavedSince("palettes", asJson(tilePalettes));

    return values || !renaming.sinceSaved().empty();
}

void TilePalettesUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
