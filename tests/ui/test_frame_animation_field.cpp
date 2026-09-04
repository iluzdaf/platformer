#include <catch2/catch_test_macros.hpp>
#include "animations/frame_animation.hpp"
#include "animations/frame_animation_data.hpp"
#include <vector>
#include <imgui_internal.h>
#include "ui/frame_animation_field.hpp"
#include "ui/data_inspector.hpp"
#include "test_helpers/headless_imgui.hpp"

TEST_CASE("A preview shows the frame the time has reached", "[AnimationPreview]")
{
    FrameAnimationData animation{{10, 11, 12}, 0.25f};

    REQUIRE(previewFrameAt(animation, 0.0, 7) == 10);
    REQUIRE(previewFrameAt(animation, 0.24, 7) == 10);
    REQUIRE(previewFrameAt(animation, 0.25, 7) == 11);
    REQUIRE(previewFrameAt(animation, 0.5, 7) == 12);
}

TEST_CASE("A preview wraps when the frames run out", "[AnimationPreview]")
{
    FrameAnimationData animation{{10, 11, 12}, 0.25f};

    REQUIRE(previewFrameAt(animation, 0.75, 7) == 10);
    REQUIRE(previewFrameAt(animation, 1.0, 7) == 11);
}

TEST_CASE("A preview of nothing stands on the tile being edited", "[AnimationPreview]")
{
    FrameAnimationData nothing;

    REQUIRE(previewFrameAt(nothing, 3.0, 42) == 42);
}

TEST_CASE(
    "A preview with no duration stands still rather than dividing by it",
    "[AnimationPreview]")
{
    FrameAnimationData stopped{{10, 11}, 0.0f};

    REQUIRE(previewFrameAt(stopped, 3.0, 42) == 42);
    REQUIRE(previewFrameAt(stopped, 0.0, 42) == 42);
}

TEST_CASE("An animation with no duration does not spin", "[AnimationPreview]")
{
    FrameAnimation stopped(FrameAnimationData{{10, 11}, 0.0f});

    stopped.update(1.0f);

    REQUIRE(stopped.getCurrentFrame() == 10);
}

#ifndef SKIP_OPENGL_TESTS

#include <glm/gtc/matrix_transform.hpp>
#include "rendering/texture2d.hpp"
#include "test_helpers/made_sheet.hpp"
#include "ui/sheet_in_scope.hpp"
#include "assets/sheet_data.hpp"

namespace
{
    float heightOfFrames(HeadlessImGui &gui, FrameAnimationData &animation, bool withSheet)
    {
        Texture2D coin = aSheetOf(7, 6);
        SheetInScope offering{&coin, SheetData{"textures/somewhere.png", glm::ivec2(16)}, 0};

        float reached = 0.0f;
        gui.frame(
            [&]
            {
                ImGui::TreeNodeSetOpen(ImGui::GetID("animation"), true);

                // "frames" is drawn inside the animation node, so its id is too
                ImGui::PushOverrideID(ImGui::GetID("animation"));
                ImGui::TreeNodeSetOpen(ImGui::GetID("frames"), true);
                ImGui::PopID();

                if (withSheet)
                {
                    ShowingSheet showing(offering);
                    inspector::draw("animation", animation);
                }
                else
                    inspector::draw("animation", animation);

                reached = ImGui::GetCurrentWindow()->DC.CursorPos.y;
            });

        return reached;
    }
}

TEST_CASE("A frame costs more room as a picture than as a number", "[FrameAnimationField]")
{
    HeadlessImGui gui;
    FrameAnimationData one{{1}, 0.1f};
    FrameAnimationData five{{1, 2, 3, 4, 5}, 0.1f};

    // the preview is the same size whatever the frames are, so it cancels out of a difference
    float asPictures = heightOfFrames(gui, five, true) - heightOfFrames(gui, one, true);
    float asNumbers = heightOfFrames(gui, five, false) - heightOfFrames(gui, one, false);

    REQUIRE(asPictures > asNumbers);
}

TEST_CASE("Frames stay what they were when nothing is picked", "[FrameAnimationField]")
{
    HeadlessImGui gui;
    FrameAnimationData animation{{1, 2, 3}, 0.1f};

    heightOfFrames(gui, animation, true);

    REQUIRE(animation.frames == std::vector<int>{1, 2, 3});
}

#endif
