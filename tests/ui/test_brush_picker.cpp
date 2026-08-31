#ifndef SKIP_OPENGL_TESTS
#include <cstddef>
#include <optional>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include "rendering/texture2d.hpp"
#include "test_helpers/asset_path.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/brush.hpp"
#include "ui/brush_picker.hpp"

namespace
{
    float heightOfPicker(
        HeadlessImGui &gui,
        const Texture2D &tileSet,
        float width,
        int tileCount,
        bool withPlayerStart = false)
    {
        std::vector<Brush> brushes;
        for (int tileIndex = 0; tileIndex < tileCount; ++tileIndex)
            brushes.push_back(Brush{Brush::Kind::Tile, tileIndex});
        if (withPlayerStart)
            brushes.push_back(Brush{Brush::Kind::PlayerStart, 0});

        float height = 0.0f;
        gui.frame(
            [&]
            {
                ImGui::BeginChild("picker", ImVec2(width, 600.0f));
                float before = ImGui::GetCursorPosY();
                drawBrushPicker(tileSet, 16, brushes, std::nullopt);
                height = ImGui::GetCursorPosY() - before;
                ImGui::EndChild();
            });

        return height;
    }

    int rowsOfPicker(
        HeadlessImGui &gui,
        const Texture2D &tileSet,
        float width,
        int tileCount,
        bool withPlayerStart = false)
    {
        float oneRow = heightOfPicker(gui, tileSet, width, 1);
        float spacing = ImGui::GetStyle().ItemSpacing.y;
        float height = heightOfPicker(gui, tileSet, width, tileCount, withPlayerStart);

        return static_cast<int>((height + spacing) / (oneRow + spacing) + 0.5f);
    }
}

TEST_CASE("The brush picker fills the width it is given", "[BrushPicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet(assetPath("textures/tile_set.png"));

    int narrow = rowsOfPicker(gui, tileSet, 200.0f, 12);
    int wide = rowsOfPicker(gui, tileSet, 400.0f, 12);

    REQUIRE(narrow > wide);
}

TEST_CASE("The brush picker puts everything on one row when it fits", "[BrushPicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet(assetPath("textures/tile_set.png"));

    REQUIRE(rowsOfPicker(gui, tileSet, 1000.0f, 12) == 1);
}

TEST_CASE("The brush picker wraps rather than overflowing a narrow panel", "[BrushPicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet(assetPath("textures/tile_set.png"));

    REQUIRE(rowsOfPicker(gui, tileSet, 100.0f, 12) > 4);
}

TEST_CASE("The player start sits in the grid with the tiles", "[BrushPicker]")
{
    HeadlessImGui gui;
    Texture2D tileSet(assetPath("textures/tile_set.png"));

    REQUIRE(rowsOfPicker(gui, tileSet, 1000.0f, 12, true) == 1);
    REQUIRE(rowsOfPicker(gui, tileSet, 200.0f, 12, true) >= rowsOfPicker(gui, tileSet, 200.0f, 12));
}
#endif // SKIP_OPENGL_TESTS
