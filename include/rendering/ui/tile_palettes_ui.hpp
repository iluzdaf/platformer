#pragma once

#include <string>
#include "rendering/ui/saveable.hpp"
#include "tile_map/tile_palette.hpp"

class Texture2D;

class TilePalettesUi
{
public:
    void draw(
        TilePalettes &tilePalettes,
        const Texture2D &tileSet,
        int tileSize,
        int &selectedTileIndex);

    void forget();

private:
    Saveable saveable;
    std::string selectedPalette;
};
