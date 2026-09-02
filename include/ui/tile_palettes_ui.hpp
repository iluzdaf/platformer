#pragma once

#include <map>
#include <optional>
#include <string>
#include "ui/armed.hpp"
#include "ui/saveable.hpp"
#include "tile_map/tile_palette.hpp"

class TextureCache;
struct EditorCommands;

struct PaletteRenamed
{
    std::string from, to;
};

std::optional<std::string> whyNotARename(
    const TilePalettes &tilePalettes,
    const std::string &from,
    const std::string &to);

std::string aNameNobodyHasTaken(const TilePalettes &tilePalettes);

void rememberRename(
    std::map<std::string, std::string> &renames,
    const std::string &from,
    const std::string &to);

class TilePalettesUi
{
public:
    std::optional<PaletteRenamed> draw(
        TilePalettes &tilePalettes,
        const TextureCache &textures,
        EditorCommands &commands,
        std::optional<Armed> &armed);

    bool hasUnsavedChanges(const TilePalettes &tilePalettes) const;
    void valuesReplaced();

private:
    void drawChooser(TilePalettes &tilePalettes);
    std::optional<PaletteRenamed> drawRename(TilePalettes &tilePalettes);
    void saveWithRenames(const TilePalettes &tilePalettes);

    Saveable saveable;
    std::string selectedPalette, askedToWarm, renamingTo;
    std::map<std::string, std::string> renames;
};
