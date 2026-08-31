#include <algorithm>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette.hpp"
#include "ui/tile_picker.hpp"

TEST_CASE("The tile picker offers a level's tiles in tile set order", "[TilePicker]")
{
    TileMap tileMap = setupTileMap(10, 10, 16, shippedPalettes().at("default"));

    std::vector<int> offered = tilesToPickFrom(tileMap);

    REQUIRE_FALSE(offered.empty());
    REQUIRE(std::is_sorted(offered.begin(), offered.end()));
}

TEST_CASE("The tile picker offers every tile the level knows about once", "[TilePicker]")
{
    TileMap tileMap = setupTileMap(10, 10, 16, shippedPalettes().at("default"));

    std::vector<int> offered = tilesToPickFrom(tileMap);

    REQUIRE(offered.size() == tileMap.getTiles().size());
    REQUIRE(std::adjacent_find(offered.begin(), offered.end()) == offered.end());
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
