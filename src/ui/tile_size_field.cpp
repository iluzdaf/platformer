#include <imgui.h>
#include "ui/marked_label.hpp"
#include "ui/tile_size_field.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_edited.hpp"
#include "tile_map/tile_palette_data.hpp"

inspector::Edited drawTileSizeField(TilePaletteData &palette)
{
    int measured = palette.tileSize.value_or(palette.tileSet.cellSize.x);
    inspector::Edited edited =
        inspector::drawAs("tileSize", measured, palette.tileSize, measured <= 0);
    if (edited)
    {
        if (measured == palette.tileSet.cellSize.x)
            palette.tileSize.reset();
        else
            palette.tileSize = measured;
    }

    if (measured <= 0)
        inspector::drawRefusal("a tile is wider than nothing");

    return edited;
}
