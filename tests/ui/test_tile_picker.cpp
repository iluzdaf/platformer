#include <algorithm>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "rendering/tile_set_textures.hpp"
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
#include <cstddef>
#include <optional>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include "rendering/texture2d.hpp"
#include "test_helpers/asset_path.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/tile_picker.hpp"
#include "ui/editor_ui.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_palette.hpp"

namespace
{
    float heightOfPicker(HeadlessImGui &gui, const Texture2D &tileSet, float width, int tileCount)
    {
        std::vector<int> tileIndices;
        for (int tileIndex = 0; tileIndex < tileCount; ++tileIndex)
            tileIndices.push_back(tileIndex);

        float height = 0.0f;
        gui.frame(
            [&]
            {
                ImGui::BeginChild("picker", ImVec2(width, 600.0f));
                float before = ImGui::GetCursorPosY();
                drawTilePicker(tileSet, 16, tileIndices, std::nullopt);
                height = ImGui::GetCursorPosY() - before;
                ImGui::EndChild();
            });

        return height;
    }

    int rowsOfPicker(HeadlessImGui &gui, const Texture2D &tileSet, float width, int tileCount)
    {
        float oneRow = heightOfPicker(gui, tileSet, width, 1);
        float height = heightOfPicker(gui, tileSet, width, tileCount);

        return static_cast<int>(height / oneRow + 0.5f);
    }
}

TEST_CASE("The tile picker offers every cell of the sheet", "[TilePicker]")
{
    Texture2D tileSet(assetPath("textures/tile_set.png"));

    std::vector<int> offered = tilesToPickFrom(tileSet, 16);

    REQUIRE(offered.size() == 49);
    REQUIRE(offered.front() == 0);
    REQUIRE(offered.back() == 48);
    REQUIRE(std::is_sorted(offered.begin(), offered.end()));
}

TEST_CASE("The tile picker offers cells the palette says nothing about", "[TilePicker]")
{
    Texture2D tileSet(assetPath("textures/tile_set.png"));

    std::vector<int> offered = tilesToPickFrom(tileSet, 16);
    const TilePalette &shipped = shippedPalettes().at("default");

    REQUIRE(offered.size() > shipped.tiles.size());
    for (int unconfigured : {6, 35, 48})
    {
        REQUIRE_FALSE(shipped.tiles.contains(unconfigured));
        REQUIRE(std::find(offered.begin(), offered.end(), unconfigured) != offered.end());
    }
}

TEST_CASE("The tile picker fills the width it is given", "[TilePicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet(assetPath("textures/tile_set.png"));

    int narrow = rowsOfPicker(gui, tileSet, 200.0f, 12);
    int wide = rowsOfPicker(gui, tileSet, 400.0f, 12);

    REQUIRE(narrow > wide);
}

TEST_CASE("The tile picker puts everything on one row when it fits", "[TilePicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet(assetPath("textures/tile_set.png"));

    REQUIRE(rowsOfPicker(gui, tileSet, 1000.0f, 12) == 1);
}

TEST_CASE("The tile picker wraps rather than overflowing a narrow panel", "[TilePicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet(assetPath("textures/tile_set.png"));

    REQUIRE(rowsOfPicker(gui, tileSet, 100.0f, 12) > 4);
}

TEST_CASE("The tile picker does not reflow when the editor gains a scrollbar", "[TilePicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet(assetPath("textures/tile_set.png"));
    const ImGuiStyle &style = ImGui::GetStyle();

    float roomy = InspectorWidth - style.WindowPadding.x * 2.0f;
    float scrolling = roomy - style.ScrollbarSize;

    REQUIRE(rowsOfPicker(gui, tileSet, roomy, 26) == rowsOfPicker(gui, tileSet, scrolling, 26));
}

#endif // SKIP_OPENGL_TESTS
