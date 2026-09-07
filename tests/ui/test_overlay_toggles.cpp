#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include "helpers/headless_imgui.hpp"
#include "ui/level_ui.hpp"
#include "ui/player_ui.hpp"

namespace
{
    template <class Draw> float rowsDrawnBy(HeadlessImGui &gui, Draw &&draw)
    {
        float reached = 0.0f;
        gui.frame(
            [&]
            {
                float top = ImGui::GetCursorScreenPos().y;
                draw();
                reached = (ImGui::GetCurrentWindow()->DC.CursorPos.y - top) /
                          ImGui::GetFrameHeightWithSpacing();
            });
        return reached;
    }
}

TEST_CASE("The player's overlay is one toggle", "[OverlayToggles]")
{
    HeadlessImGui gui;
    PlayerUi playerUi;

    REQUIRE(rowsDrawnBy(gui, [&] { playerUi.drawOverlayToggles(); }) == 1.0f);
}

TEST_CASE("The level's overlays are the tile map and the navigation's", "[OverlayToggles]")
{
    HeadlessImGui gui;
    LevelUi levelUi;

    REQUIRE(rowsDrawnBy(gui, [&] { levelUi.drawOverlayToggles(); }) == 2.0f);
}
