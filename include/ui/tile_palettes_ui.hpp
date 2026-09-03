#pragma once

#include <map>
#include <optional>
#include <string>
#include "ui/armed.hpp"
#include "ui/saveable.hpp"
#include "ui/palette_renamed.hpp"
#include "tile_map/tile_palette.hpp"

class TextureCache;
struct EditorCommands;

class TilePalettesUi
{
public:
    std::optional<PaletteRenamed> draw(
        TilePalettes &tilePalettes,
        const TextureCache &textures,
        EditorCommands &commands,
        std::optional<Armed> &armed);

    void save(TilePalettes &tilePalettes);
    void revert(TilePalettes &tilePalettes);
    bool unsavedSince(const TilePalettes &tilePalettes);
    void valuesReplaced();

private:
    void drawChooser(TilePalettes &tilePalettes);
    std::optional<PaletteRenamed> drawRename(TilePalettes &tilePalettes);
    void saveWithRenames(const TilePalettes &tilePalettes);
    void forgetUnsavedRenaming();

    Saveable saveable;
    std::string selectedPalette, askedToWarm, renamingTo;
    std::map<std::string, std::string> renames;
};
