#include <catch2/catch_test_macros.hpp>
#include "rendering/tile_set_fit.hpp"
#include "ui/tile_picker.hpp"

TEST_CASE("A sheet holds what both its axes divide into", "[TilePicker]")
{
    REQUIRE(tilesInSheet(112, 112, 16) == 49);
    REQUIRE(tilesInSheet(112, 128, 16) == 56);
    REQUIRE(tilesInSheet(128, 128, 16) == 64);
    REQUIRE(tilesInSheet(96, 96, 16) == 36);
}

TEST_CASE("A sheet too small for one tile holds none", "[TilePicker]")
{
    REQUIRE(tilesInSheet(8, 8, 16) == 0);
    REQUIRE(tilesInSheet(112, 112, 0) == 0);
}

#ifndef SKIP_OPENGL_TESTS
#include <algorithm>
#include <cstddef>
#include <optional>
#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include "rendering/texture2d.hpp"
#include "test_helpers/made_sheet.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/tile_picker.hpp"
#include "ui/editor_ui.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_palette.hpp"

namespace
{
    float heightOfPicker(HeadlessImGui &gui, const Texture2D &sheet, float width, int cellSize)
    {
        float height = 0.0f;
        gui.frame(
            [&]
            {
                ImGui::BeginChild("picker", ImVec2(width, 900.0f));
                float before = ImGui::GetCursorPosY();
                drawTilePicker(sheet, Sheet{"", glm::ivec2(cellSize)}, std::nullopt);
                height = ImGui::GetCursorPosY() - before;
                ImGui::EndChild();
            });

        return height;
    }

    int rowsOfPicker(HeadlessImGui &gui, const Texture2D &sheet, float width, int cellSize)
    {
        int oneCell = static_cast<int>(std::min(sheet.getWidth(), sheet.getHeight()));
        float oneRow = heightOfPicker(gui, sheet, 1000.0f, oneCell);
        float height = heightOfPicker(gui, sheet, width, cellSize);

        return static_cast<int>(height / oneRow + 0.5f);
    }
}

TEST_CASE("The tile picker offers more cells than the palette configures", "[TilePicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet = aSheetOf(7, 6);
    const TilePalette &palette = shippedPalettes().at("default");

    int cells = tilesInSheet(
        static_cast<int>(tileSet.getWidth()),
        static_cast<int>(tileSet.getHeight()),
        palette.tileSet.cellSize.x);

    REQUIRE(cells > static_cast<int>(palette.tiles.size()));
}

TEST_CASE("A smaller cell means the picker draws more of them", "[TilePicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet = aSheetOf(7, 6);

    float few = heightOfPicker(gui, tileSet, 200.0f, 32);
    float many = heightOfPicker(gui, tileSet, 200.0f, 8);

    REQUIRE(many > few);
}

TEST_CASE("The tile picker fills the width it is given", "[TilePicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet = aSheetOf(7, 6);

    int narrow = rowsOfPicker(gui, tileSet, 200.0f, 16);
    int wide = rowsOfPicker(gui, tileSet, 400.0f, 16);

    REQUIRE(narrow > wide);
}

TEST_CASE("The tile picker puts everything on one row when it fits", "[TilePicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet = aSheetOf(7, 6);

    REQUIRE(rowsOfPicker(gui, tileSet, 1000.0f, 32) == 1);
}

TEST_CASE("The tile picker wraps rather than overflowing a narrow panel", "[TilePicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet = aSheetOf(7, 6);

    REQUIRE(rowsOfPicker(gui, tileSet, 100.0f, 16) > 4);
}

TEST_CASE("The tile picker does not reflow when the editor gains a scrollbar", "[TilePicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet = aSheetOf(7, 6);
    const ImGuiStyle &style = ImGui::GetStyle();

    float roomy = InspectorWidth - style.WindowPadding.x * 2.0f;
    float scrolling = roomy - style.ScrollbarSize;

    REQUIRE(rowsOfPicker(gui, tileSet, roomy, 16) == rowsOfPicker(gui, tileSet, scrolling, 16));
}

#endif // SKIP_OPENGL_TESTS
