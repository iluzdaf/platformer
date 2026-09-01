#pragma once

#include <optional>
#include <string>
#include "ui/armed.hpp"
#include "ui/saveable.hpp"
#include "tile_map/tile_palette.hpp"

class TextureCache;
struct EditorCommands;

class TilePalettesUi
{
public:
    void draw(
        TilePalettes &tilePalettes,
        const TextureCache &textures,
        EditorCommands &commands,
        std::optional<Armed> &armed);

    std::optional<std::string> cannotSave(
        const TilePalettes &tilePalettes,
        const TextureCache &textures) const;

    bool hasUnsavedChanges(const TilePalettes &tilePalettes) const;
    void valuesReplaced();

private:
    Saveable saveable;
    std::string selectedPalette, askedToWarm;
};
