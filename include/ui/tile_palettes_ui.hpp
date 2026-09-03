#pragma once

#include <optional>
#include <string>
#include "ui/armed.hpp"
#include "ui/saveable.hpp"
#include "ui/renaming.hpp"
#include "tile_map/tile_palette.hpp"

class TextureCache;
struct EditorCommands;

class TilePalettesUi
{
public:
    std::optional<Renamed> draw(
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
    std::optional<Renamed> drawRename(TilePalettes &tilePalettes);

    Saveable saveable;
    Renaming renaming;
    std::string selectedPalette, askedToWarm;
};

std::string aNameNobodyHasTaken(const TilePalettes &tilePalettes);
