#include <catch2/catch_test_macros.hpp>
#include "animations/frame_animation_data.hpp"
#include <vector>
#include <imgui_internal.h>
#include "ui/data_inspector.hpp"
#include "ui/inspector_edited.hpp"
#include "helpers/headless_imgui.hpp"

#ifndef SKIP_OPENGL_TESTS

#include <glm/gtc/matrix_transform.hpp>
#include "rendering/texture2d.hpp"
#include "helpers/made_sheet.hpp"
#include "ui/in_scope.hpp"
#include "ui/sheet_in_scope.hpp"
#include "assets/sheet_data.hpp"
#include "helpers/pictures_drawn.hpp"
#include "ui/tile_picker.hpp"

namespace
{
    bool drawsFramesAsPictures(HeadlessImGui &gui, FrameAnimationData &animation, bool withSheet)
    {
        Texture2D coin = aSheetOf(7, 6);
        SheetInScope offering{&coin, SheetData{"textures/somewhere.png", glm::ivec2(16)}};

        return drawsAPictureWide(
            gui,
            TilePickerCellSize,
            [&]
            {
                ImGui::TreeNodeSetOpen(ImGui::GetID("animation"), true);

                ImGui::PushOverrideID(ImGui::GetID("animation"));
                ImGui::TreeNodeSetOpen(ImGui::GetID("frames"), true);
                ImGui::PopID();

                if (withSheet)
                {
                    InScope showing(offering);
                    inspector::draw("animation", animation);
                }
                else
                    inspector::draw("animation", animation);
            });
    }
}

TEST_CASE(
    "Frames are pictures to pick when a sheet is in scope, and numbers otherwise",
    "[FrameAnimationField]")
{
    HeadlessImGui gui;
    FrameAnimationData animation{{1, 2, 3}, 0.1f};

    REQUIRE(drawsFramesAsPictures(gui, animation, true));
    REQUIRE_FALSE(drawsFramesAsPictures(gui, animation, false));
}

TEST_CASE("Frames stay what they were when nothing is picked", "[FrameAnimationField]")
{
    HeadlessImGui gui;
    FrameAnimationData animation{{1, 2, 3}, 0.1f};

    drawsFramesAsPictures(gui, animation, true);

    REQUIRE(animation.frames == std::vector<int>{1, 2, 3});
}

TEST_CASE("Cues are drawn beside the frames and left as they were", "[FrameAnimationField]")
{
    HeadlessImGui gui;
    FrameAnimationData animation{{0, 1, 2}, 0.1f, {{1, "onSwing"}}};

    inspector::Edited edited;
    gui.frame(
        [&]
        {
            ImGui::TreeNodeSetOpen(ImGui::GetID("animation"), true);
            ImGui::PushOverrideID(ImGui::GetID("animation"));
            ImGui::TreeNodeSetOpen(ImGui::GetID("cues"), true);
            ImGui::PopID();
            edited = inspector::draw("animation", animation);
        });

    REQUIRE_FALSE(edited.whileEditing);
    REQUIRE_FALSE(edited.onCommit);
    REQUIRE(animation.cues == std::vector<FrameCueData>{{1, "onSwing"}});
}

#endif
