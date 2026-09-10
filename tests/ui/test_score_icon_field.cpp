#include <catch2/catch_test_macros.hpp>
#include <string>
#include <imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include "assets/sheet_data.hpp"
#include "game/score_icon_data.hpp"
#include "helpers/headless_imgui.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/score_icon_field.hpp"

TEST_CASE("A score icon draws itself rather than falling through", "[ScoreIconField]")
{
    STATIC_REQUIRE(inspector::HasCustomField<ScoreIconData>);
}

#ifndef SKIP_OPENGL_TESTS

#include "rendering/texture2d.hpp"
#include "helpers/made_sheet.hpp"
#include "ui/sheet_in_scope.hpp"
#include "helpers/pictures_drawn.hpp"
#include "ui/tile_picker.hpp"

namespace
{
    bool drawsTheFrameAsAPicture(HeadlessImGui &gui, ScoreIconData &icon, bool withSheet)
    {
        Texture2D sheet = aSheetOf(7, 6);
        SheetInScope offering{&sheet, icon.sheet};

        return drawsAPictureWide(
            gui,
            TilePickerCellSize,
            [&]
            {
                ImGui::TreeNodeSetOpen(ImGui::GetID("scoreIcon"), true);

                if (withSheet)
                {
                    ShowingSheet showing(offering);
                    inspector::draw("scoreIcon", icon);
                }
                else
                    inspector::draw("scoreIcon", icon);
            });
    }
}

TEST_CASE(
    "The frame is a picture to pick when a sheet is in scope, and a number otherwise",
    "[ScoreIconField]")
{
    HeadlessImGui gui;
    ScoreIconData icon{SheetData{"textures/somewhere.png", glm::ivec2(16)}, 3};

    REQUIRE(drawsTheFrameAsAPicture(gui, icon, true));
    REQUIRE_FALSE(drawsTheFrameAsAPicture(gui, icon, false));
}

TEST_CASE("The frame keeps what it was when nothing is picked", "[ScoreIconField]")
{
    HeadlessImGui gui;
    ScoreIconData icon{SheetData{"textures/somewhere.png", glm::ivec2(16)}, 3};

    drawsTheFrameAsAPicture(gui, icon, true);

    REQUIRE(icon.frame == 3);
    REQUIRE(icon.sheet.texture.path == "textures/somewhere.png");
}

#endif
