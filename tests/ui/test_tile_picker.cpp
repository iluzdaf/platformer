#ifndef SKIP_OPENGL_TESTS
#include <cstddef>
#include <numeric>
#include <optional>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include "rendering/texture2d.hpp"
#include "test_helpers/asset_path.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/tile_picker.hpp"

namespace
{
    float heightOfPicker(HeadlessImGui &gui, const Texture2D &tileSet, float width, int tileCount)
    {
        std::vector<int> tileIndices(static_cast<std::size_t>(tileCount));
        std::iota(tileIndices.begin(), tileIndices.end(), 0);

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
        float spacing = ImGui::GetStyle().ItemSpacing.y;
        float height = heightOfPicker(gui, tileSet, width, tileCount);

        return static_cast<int>((height + spacing) / (oneRow + spacing) + 0.5f);
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
#endif // SKIP_OPENGL_TESTS
