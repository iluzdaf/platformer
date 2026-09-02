#include <cfloat>
#include <algorithm>
#include <array>
#include <cstddef>
#include <tuple>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/tile_palettes_ui.hpp"
#include "ui/save_controls.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/armed.hpp"
#include "ui/tile_picker.hpp"
#include "ui/tile_field_context.hpp"
#include "rendering/tile_set_textures.hpp"
#include "tile_map/tile_data.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "assets/asset_paths.hpp"
#include "tile_map/tile_palette.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/texture_cache.hpp"
#include "ui/editor_commands.hpp"
#include "tile_map/tile_set.hpp"

std::optional<std::string> whyNotARename(
    const TilePalettes &tilePalettes,
    const std::string &from,
    const std::string &to)
{
    if (to.empty())
        return "a palette needs a name";

    if (to == from)
        return std::nullopt;

    if (tilePalettes.contains(to))
        return "there is already a palette called \"" + to + "\"";

    return std::nullopt;
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

void rememberRename(
    std::map<std::string, std::string> &renames,
    const std::string &from,
    const std::string &to)
{
    std::string onDisk = from;
    for (const auto &[was, is] : renames)
        if (is == from)
            onDisk = was;

    if (onDisk == to)
        renames.erase(onDisk);
    else
        renames.insert_or_assign(onDisk, to);
}

void TilePalettesUi::saveWithRenames(const TilePalettes &tilePalettes)
{
    saveTilePalettes(tilePalettes);

    if (renames.empty())
        return;

    renamePaletteInLevels(std::string(assets::Levels), renames);
    renames.clear();
}

namespace
{
    constexpr float RenameWidth = 62.0f;

    bool drawNameField(std::string &name)
    {
        std::array<char, 256> buffer{};
        name.copy(buffer.data(), std::min(name.size(), buffer.size() - 1));

        ImGui::TextUnformatted("name");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-RenameWidth);
        if (ImGui::InputText("##name", buffer.data(), buffer.size()))
            name = buffer.data();

        return true;
    }
}

void TilePalettesUi::drawChooser(TilePalettes &tilePalettes)
{
    ImGui::SetNextItemWidth(-RenameWidth);
    if (ImGui::BeginCombo("##palette", selectedPalette.c_str()))
    {
        for (const auto &[name, palette] : tilePalettes)
            if (ImGui::Selectable(name.c_str(), name == selectedPalette))
            {
                selectedPalette = name;
                renamingTo.clear();
            }

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
    renamingTo.clear();
}

std::optional<PaletteRenamed> TilePalettesUi::drawRename(TilePalettes &tilePalettes)
{
    if (renamingTo.empty())
        renamingTo = selectedPalette;

    std::ignore = drawNameField(renamingTo);

    std::optional<std::string> why = whyNotARename(tilePalettes, selectedPalette, renamingTo);
    ImGui::SameLine();
    ImGui::BeginDisabled(why.has_value() || renamingTo == selectedPalette);
    bool rename = ImGui::Button("rename", ImVec2(-FLT_MIN, 0.0f));
    ImGui::EndDisabled();

    if (why)
        ImGui::TextColored(CannotSaveColour, "%s", why->c_str());

    if (!rename)
        return std::nullopt;

    PaletteRenamed renamed{selectedPalette, renamingTo};
    auto node = tilePalettes.extract(selectedPalette);
    node.key() = renamingTo;
    tilePalettes.insert(std::move(node));

    rememberRename(renames, renamed.from, renamed.to);
    selectedPalette = renamed.to;

    return renamed;
}

std::optional<PaletteRenamed> TilePalettesUi::draw(
    TilePalettes &tilePalettes,
    const TextureCache &textures,
    EditorCommands &commands,
    std::optional<Armed> &armed)
{
    drawSaveControls(
        saveable,
        "palettes",
        tilePalettes,
        [this](const TilePalettes &saving) { saveWithRenames(saving); });
    ImGui::Separator();

    if (!tilePalettes.empty() && !tilePalettes.contains(selectedPalette))
        selectedPalette = tilePalettes.begin()->first;

    drawChooser(tilePalettes);

    if (tilePalettes.empty())
    {
        ImGui::TextDisabled("no palettes");
        return std::nullopt;
    }

    ImGui::Separator();
    if (std::optional<PaletteRenamed> renamed = drawRename(tilePalettes))
        return renamed;

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
        return std::nullopt;
    }

    int cells = tilesInSheet(
        static_cast<int>(tileSet->getWidth()),
        static_cast<int>(tileSet->getHeight()),
        palette.tileSet.tileSize);
    if (cells <= 0)
    {
        ImGui::TextDisabled("no whole tiles in this tile set");
        return std::nullopt;
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
        return std::nullopt;
    }

    ImGui::Text("tile %d", *picked);

    ShowingTilesFrom offering(TileFieldContext{tileSet, palette.tileSet});

    auto known = palette.tiles.find(*picked);
    if (known != palette.tiles.end())
    {
        inspector::drawFields(known->second);
        return std::nullopt;
    }

    TileData nothingSaid;
    if (inspector::drawFields(nothingSaid))
        palette.tiles.insert({*picked, nothingSaid});

    return std::nullopt;
}

bool TilePalettesUi::hasUnsavedChanges(const TilePalettes &tilePalettes) const
{
    return saveable.unsaved("palettes", asJson(tilePalettes));
}

void TilePalettesUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
