#include <catch2/catch_test_macros.hpp>
#include "animations/frame_animation.hpp"
#include "animations/frame_animation_data.hpp"
#include "ui/frame_animation_field.hpp"

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
