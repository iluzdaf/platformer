#include <catch2/catch_test_macros.hpp>
#include <imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include "assets/sheet_data.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/sheet_field.hpp"

namespace
{
    float squareHeightOf(HeadlessImGui &gui, SheetData &sheet)
    {
        float reached = 0.0f;
        gui.frame(
            [&]
            {
                drawSquareSheetFields(sheet);
                reached = ImGui::GetCurrentWindow()->DC.CursorPos.y;
            });

        return reached;
    }

    float drawnHeightOf(HeadlessImGui &gui, SheetData &sheet)
    {
        float reached = 0.0f;
        gui.frame(
            [&]
            {
                ImGui::TreeNodeSetOpen(ImGui::GetID("sheet"), true);
                inspector::draw("sheet", sheet);
                reached = ImGui::GetCurrentWindow()->DC.CursorPos.y;
            });

        return reached;
    }
}

TEST_CASE("A sheet draws itself rather than falling through", "[SheetField]")
{
    STATIC_REQUIRE(inspector::HasCustomField<SheetData>);
}

TEST_CASE("A sheet naming no texture says so where the texture is", "[SheetField]")
{
    HeadlessImGui gui;

    SheetData named{"textures/player.png", glm::ivec2(16)};
    SheetData unnamed{"", glm::ivec2(16)};

    float withTexture = drawnHeightOf(gui, named);
    float without = drawnHeightOf(gui, unnamed);

    REQUIRE(without > withTexture);
}

TEST_CASE("A sheet naming a texture nobody has says so where the texture is", "[SheetField]")
{
    HeadlessImGui gui;

    SheetData onDisk{"textures/player.png", glm::ivec2(16)};
    SheetData nowhere{"textures/somewhere.png", glm::ivec2(16)};

    float found = drawnHeightOf(gui, onDisk);
    float missing = drawnHeightOf(gui, nowhere);

    REQUIRE(missing > found);
}

TEST_CASE("A sheet keeps what it was given after being drawn", "[SheetField]")
{
    HeadlessImGui gui;
    SheetData sheet{"textures/somewhere.png", glm::ivec2(24, 32)};

    drawnHeightOf(gui, sheet);

    REQUIRE(sheet.texture == "textures/somewhere.png");
    REQUIRE(sheet.cellSize == glm::ivec2(24, 32));
}

TEST_CASE("Square sheet fields say when the cells are not square", "[SheetField]")
{
    HeadlessImGui gui;

    SheetData square{"textures/player.png", glm::ivec2(16)};
    SheetData oblong{"textures/player.png", glm::ivec2(16, 24)};

    REQUIRE(squareHeightOf(gui, oblong) > squareHeightOf(gui, square));
}

TEST_CASE("Nudging the one cell size squares the cells", "[SheetField]")
{
    HeadlessImGui gui;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    SheetData sheet{"textures/player.png", glm::ivec2(16, 24)};
    auto drawing = [&] { drawSquareSheetFields(sheet); };

    gui.frame(
        [&]
        {
            ImGui::ActivateItemByID(ImGui::GetID("##cellSize"));
            drawing();
        });
    gui.frame(drawing);

    ImGuiIO &io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiKey_RightArrow, true);
    gui.frame(drawing);
    io.AddKeyEvent(ImGuiKey_RightArrow, false);
    gui.frame(drawing);

    REQUIRE(sheet.cellSize.x == sheet.cellSize.y);
    REQUIRE(sheet.cellSize.x > 16);
}
