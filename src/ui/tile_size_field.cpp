#include <imgui.h>
#include "ui/tile_size_field.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/unsaved_colours.hpp"
#include "tile_map/tile_palette_data.hpp"

inspector::Edited drawTileSizeField(TilePaletteData &palette)
{
    int measured = palette.tileSize.value_or(palette.tileSet.cellSize.x);
    inspector::Edited edited = inspector::draw("tileSize", measured);
    if (edited)
    {
        if (measured == palette.tileSet.cellSize.x)
            palette.tileSize.reset();
        else
            palette.tileSize = measured;
    }

    if (measured <= 0)
        ImGui::TextColored(CannotSaveColour, "a tile is wider than nothing");

    return edited;
}
