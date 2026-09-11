#include <cfloat>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/tile_palettes_ui.hpp"
#include "ui/saved_in_scope.hpp"
#include <exception>
#include "tile_map/tile.hpp"
#include "ui/renaming.hpp"
#include "ui/level_rewriting.hpp"
#include "ui/renames.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/tile_size_field.hpp"
#include "ui/armed.hpp"
#include "ui/tile_picker.hpp"
#include "ui/sheet_in_scope.hpp"
#include "ui/sheet_field.hpp"
#include "ui/sheet_preview.hpp"
#include "rendering/tile_set_fit.hpp"
#include "tile_map/tile_data.hpp"
#include "game/level_data.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/texture_cache.hpp"
#include "ui/editor_commands.hpp"
#include "ui/inspector_edited.hpp"
#include "assets/sheet_data.hpp"

namespace
{
    std::optional<std::string> aTileThatCannotBeBuilt(const TilePalettes &tilePalettes)
    {
        for (const auto &[name, palette] : tilePalettes)
        {
            int tileSize = tileSizeOf(palette);
            if (tileSize <= 0)
                return "palette \"" + name + "\" measures its tiles at " +
                       std::to_string(tileSize) + ", and a tile is wider than nothing";

            for (const auto &[index, tile] : palette.tiles)
            {
                try
                {
                    Tile built(tile, glm::vec2(static_cast<float>(tileSize)));
                }
                catch (const std::exception &e)
                {
                    return "palette \"" + name + "\" tile " + std::to_string(index) + ": " +
                           e.what();
                }
            }
        }

        return std::nullopt;
    }

    constexpr float ButtonsWidth = 124.0f;
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

TilePalettesUi::TilePalettesUi(std::string levelsDirectory, WritePalettes writePalettes)
    : levelsDirectory(std::move(levelsDirectory)), writePalettes(std::move(writePalettes))
{
}

bool TilePalettesUi::shownIn(const TilePalettes &tilePalettes, const std::string &name) const
{
    return tilePalettes.contains(name) && !renaming.gone(name);
}

std::optional<std::string> TilePalettesUi::firstRemainingAfter(
    const TilePalettes &tilePalettes,
    const std::string &leaving) const
{
    for (const auto &[name, palette] : tilePalettes)
        if (name != leaving && !renaming.gone(name))
            return renaming.shownName(name);

    return std::nullopt;
}

std::string TilePalettesUi::firstShownIn(const TilePalettes &tilePalettes) const
{
    for (const auto &[name, palette] : tilePalettes)
        if (!renaming.gone(name))
            return name;

    return {};
}

void TilePalettesUi::show(const std::string &palette)
{
    selectedPalette = palette;
}

const std::string &TilePalettesUi::shownPalette() const
{
    return selectedPalette;
}

void TilePalettesUi::add(TilePalettes &tilePalettes)
{
    std::string name = aNameNobodyHasTaken(tilePalettes);
    TilePaletteData made;
    if (tilePalettes.contains(selectedPalette))
        made.tileSet = tilePalettes.at(selectedPalette).tileSet;

    tilePalettes.insert({name, made});
    renaming.added(name);
    namesTouched = true;
    selectedPalette = name;
}

void TilePalettesUi::remove(TilePalettes &tilePalettes)
{
    if (!shownIn(tilePalettes, selectedPalette))
        return;

    if (renaming.remove(selectedPalette, firstRemainingAfter(tilePalettes, selectedPalette)))
        lookAheadAtLevels(
            renaming,
            levelsDirectory,
            [](LevelData &levelData, const Renames &renames)
            { return rewriting::paletteIn(levelData.tileMapData, renames); });
    else
        tilePalettes.erase(selectedPalette);

    namesTouched = true;
    selectedPalette = firstShownIn(tilePalettes);
}

bool TilePalettesUi::namesChanged()
{
    return std::exchange(namesTouched, false);
}

void TilePalettesUi::drawChooser(TilePalettes &tilePalettes)
{
    ImGui::SetNextItemWidth(-ButtonsWidth);
    if (ImGui::BeginCombo("##palette", renaming.shownName(selectedPalette).c_str()))
    {
        for (const auto &[name, palette] : tilePalettes)
            if (!renaming.gone(name) &&
                ImGui::Selectable(renaming.shownName(name).c_str(), name == selectedPalette))
                selectedPalette = name;

        ImGui::EndCombo();
    }

    ImGui::SameLine();
    if (ImGui::Button("add"))
        add(tilePalettes);

    ImGui::SameLine();
    ImGui::BeginDisabled(selectedPalette.empty());
    if (ImGui::Button("remove", ImVec2(-FLT_MIN, 0.0f)))
        remove(tilePalettes);

    ImGui::EndDisabled();
}

void TilePalettesUi::drawRename(const TilePalettes &tilePalettes)
{
    if (!renaming.draw(
            "a palette",
            selectedPalette,
            [this, &tilePalettes](const std::string &name)
            { return shownIn(tilePalettes, name) || renaming.somethingIsBecoming(name); }))
        return;

    namesTouched = true;
    lookAheadAtLevels(
        renaming,
        levelsDirectory,
        [](LevelData &levelData, const Renames &renames)
        { return rewriting::paletteIn(levelData.tileMapData, renames); });
}

void TilePalettesUi::draw(
    TilePalettes &tilePalettes,
    const TextureCache &textures,
    EditorCommands &commands,
    std::optional<Armed> &armed)
{
    if (!shownIn(tilePalettes, selectedPalette))
        selectedPalette = firstShownIn(tilePalettes);

    drawChooser(tilePalettes);

    if (selectedPalette.empty())
    {
        ImGui::TextDisabled("no palettes");
        return;
    }

    ImGui::Separator();
    drawRename(tilePalettes);

    SavedInScope was(asItWasSaved<TilePalettes>(saveable.lastSeen("palettes")));
    inspector::InField shown(renaming.shownName(selectedPalette));

    TilePaletteData &palette = tilePalettes.at(selectedPalette);
    inspector::Edited edited = drawSquareSheetFields(palette.tileSet);
    edited |= drawTileSizeField(palette);
    ImGui::Separator();

    edited |= drawTiles(palette, textures, commands, armed);
    if (edited.onCommit)
        commands.onPalettesChanged();
}

inspector::Edited TilePalettesUi::drawTiles(
    TilePaletteData &palette,
    const TextureCache &textures,
    EditorCommands &commands,
    std::optional<Armed> &armed)
{

    const Texture2D *tileSet = textures.find(palette.tileSet.texture.path);
    if (!tileSet)
    {
        if (!palette.tileSet.texture.path.empty() && palette.tileSet.texture.path != askedToWarm)
        {
            askedToWarm = palette.tileSet.texture.path;
            commands.onWarmTexture(palette.tileSet.texture.path);
        }

        ImGui::TextDisabled("no texture at %s", palette.tileSet.texture.path.c_str());
        return {};
    }

    int cells = tilesInSheet(
        static_cast<int>(tileSet->getWidth()),
        static_cast<int>(tileSet->getHeight()),
        palette.tileSet.cellSize.x);
    if (cells <= 0)
    {
        ImGui::TextDisabled("no whole tiles in this tile set");
        return {};
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
        return {};
    }

    ImGui::Text("tile %d", *picked);

    SheetInScope scope{tileSet, palette.tileSet};
    ShowingSheet offering(scope);

    auto known = palette.tiles.find(*picked);
    if (known != palette.tiles.end())
    {
        drawTilePreview(scope, *picked, known->second, tileSizeOf(palette));
        return inspector::drawFields(known->second);
    }

    TileData nothingSaid;
    drawTilePreview(scope, *picked, nothingSaid, tileSizeOf(palette));
    inspector::Edited said = inspector::drawFields(nothingSaid);
    if (said)
        palette.tiles.insert({*picked, nothingSaid});

    return said;
}
void TilePalettesUi::revert(TilePalettes &tilePalettes)
{
    revertTo(saveable, "palettes", tilePalettes, renaming);
}

bool TilePalettesUi::save(TilePalettes &tilePalettes, LevelData &playing)
{
    Renames pending = renaming.sinceSaved();
    std::vector<std::string> removed = renaming.removed();

    if (!writeRenamesIntoLevels(
            renaming,
            levelsDirectory,
            [](LevelData &levelData, const Renames &renames)
            { return rewriting::paletteIn(levelData.tileMapData, renames); }))
        return false;

    for (const std::string &name : removed)
        tilePalettes.erase(name);

    renamesTakeEffect(pending, tilePalettes);
    selectedPalette = nameAfterRenames(pending, selectedPalette);

    writePalettes(tilePalettes);
    saveable.saved("palettes", asJson(tilePalettes));

    return rewriting::paletteIn(playing.tileMapData, pending);
}

bool TilePalettesUi::unsavedSince(const TilePalettes &tilePalettes)
{
    bool values = saveable.unsavedSince("palettes", asJson(tilePalettes));

    return values || renaming.pending();
}

std::optional<std::string> TilePalettesUi::cannotSaveBecause(const TilePalettes &tilePalettes)
{
    if (std::optional<std::string> renames = renaming.cannotSaveBecause())
        return renames;

    return tileGate.to(asJson(tilePalettes), [&] { return aTileThatCannotBeBuilt(tilePalettes); });
}

bool TilePalettesUi::reloaded(TilePalettes &current, const TilePalettes &onDisk)
{
    return reload(saveable, "palettes", current, onDisk);
}
