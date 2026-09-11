#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include "helpers/headless_imgui.hpp"
#include "helpers/palettes.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "ui/tile_size_field.hpp"

namespace
{
    TilePaletteData aPaletteWithCellsOf(int cell)
    {
        TilePaletteData palette = paletteOf({{0, TileData{}}});
        palette.tileSet.cellSize = glm::ivec2(cell);
        return palette;
    }

    void nudge(HeadlessImGui &gui, TilePaletteData &palette, ImGuiKey key)
    {
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        auto drawing = [&] { drawTileSizeField(palette); };

        gui.frame(
            [&]
            {
                ImGui::ActivateItemByID(ImGui::GetID("##tileSize"));
                drawing();
            });
        gui.frame(drawing);

        ImGuiIO &io = ImGui::GetIO();
        io.AddKeyEvent(key, true);
        gui.frame(drawing);
        io.AddKeyEvent(key, false);
        gui.frame(drawing);
    }
}

TEST_CASE("A tile left the size of its cells is not written down", "[TileSizeField]")
{
    HeadlessImGui gui;
    TilePaletteData palette = aPaletteWithCellsOf(16);

    gui.frame([&] { drawTileSizeField(palette); });

    REQUIRE_FALSE(palette.tileSize.has_value());
}

TEST_CASE("Looking at a palette does not rewrite a tile size it already had", "[TileSizeField]")
{
    HeadlessImGui gui;
    TilePaletteData palette = aPaletteWithCellsOf(16);
    palette.tileSize = 16;

    gui.frame([&] { drawTileSizeField(palette); });

    REQUIRE(palette.tileSize == 16);
}

TEST_CASE("A tile size shows the cells until it is measured apart", "[TileSizeField]")
{
    HeadlessImGui gui;
    TilePaletteData palette = aPaletteWithCellsOf(32);

    nudge(gui, palette, ImGuiKey_RightArrow);

    REQUIRE(palette.tileSize.has_value());
    REQUIRE(*palette.tileSize > 32);
}

TEST_CASE("A tile size nudged back onto its cells is forgotten", "[TileSizeField]")
{
    HeadlessImGui gui;
    TilePaletteData palette = aPaletteWithCellsOf(32);
    palette.tileSize = 33;

    nudge(gui, palette, ImGuiKey_LeftArrow);

    REQUIRE_FALSE(palette.tileSize.has_value());
}
