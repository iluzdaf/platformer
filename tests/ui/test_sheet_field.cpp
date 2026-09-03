#include <catch2/catch_test_macros.hpp>
#include <imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include "assets/sheet.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/sheet_field.hpp"

namespace
{
    float drawnHeightOf(HeadlessImGui &gui, Sheet &sheet)
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
    STATIC_REQUIRE(inspector::HasCustomField<Sheet>);
}

TEST_CASE("A sheet naming no texture says so where the texture is", "[SheetField]")
{
    HeadlessImGui gui;

    Sheet named{"textures/somewhere.png", glm::ivec2(16)};
    Sheet unnamed{"", glm::ivec2(16)};

    float withTexture = drawnHeightOf(gui, named);
    float without = drawnHeightOf(gui, unnamed);

    REQUIRE(without > withTexture);
}

TEST_CASE("A sheet keeps what it was given after being drawn", "[SheetField]")
{
    HeadlessImGui gui;
    Sheet sheet{"textures/somewhere.png", glm::ivec2(24, 32)};

    drawnHeightOf(gui, sheet);

    REQUIRE(sheet.texture == "textures/somewhere.png");
    REQUIRE(sheet.cellSize == glm::ivec2(24, 32));
}
