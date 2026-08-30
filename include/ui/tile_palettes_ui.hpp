#pragma once

#include <optional>
#include <string>
#include "ui/brush.hpp"
#include "ui/saveable.hpp"
#include "tile_map/tile_palette.hpp"

class Texture2D;

class TilePalettesUi
{
public:
    void draw(
        TilePalettes &tilePalettes,
        const Texture2D &tileSet,
        int tileSize,
        std::optional<Brush> &brush);

    void valuesReplaced();

private:
    Saveable saveable;
    std::string selectedPalette;
};
