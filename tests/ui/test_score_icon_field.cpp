#include <catch2/catch_test_macros.hpp>
#include <string>
#include <imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include "assets/sheet_data.hpp"
#include "game/score_icon_data.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/score_icon_field.hpp"

TEST_CASE("A score icon draws itself rather than falling through", "[ScoreIconField]")
{
    STATIC_REQUIRE(inspector::HasCustomField<ScoreIconData>);
}

#ifndef SKIP_OPENGL_TESTS

#include "rendering/texture2d.hpp"
#include "test_helpers/made_sheet.hpp"
#include "ui/sheet_in_scope.hpp"

namespace
{
    float heightOfIcon(HeadlessImGui &gui, ScoreIconData &icon, bool withSheet)
    {
        Texture2D sheet = aSheetOf(7, 6);
        SheetInScope offering{&sheet, icon.sheet};

        float reached = 0.0f;
        gui.frame(
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

                reached = ImGui::GetCurrentWindow()->DC.CursorPos.y;
            });

        return reached;
    }
}

TEST_CASE("The frame is a picture to pick when a sheet is in scope", "[ScoreIconField]")
{
    HeadlessImGui gui;
    ScoreIconData icon{SheetData{"textures/somewhere.png", glm::ivec2(16)}, 3};

    float asNumber = heightOfIcon(gui, icon, false);
    float asPicture = heightOfIcon(gui, icon, true);

    REQUIRE(asPicture > asNumber);
}

TEST_CASE("The frame keeps what it was when nothing is picked", "[ScoreIconField]")
{
    HeadlessImGui gui;
    ScoreIconData icon{SheetData{"textures/somewhere.png", glm::ivec2(16)}, 3};

    heightOfIcon(gui, icon, true);

    REQUIRE(icon.frame == 3);
    REQUIRE(icon.sheet.texture == "textures/somewhere.png");
}

#endif
