#include <catch2/catch_test_macros.hpp>
#include "animations/frame_animation.hpp"
#include "animations/frame_animation_data.hpp"

TEST_CASE("Default Constucted FrameAnimation behaves correctly", "[FrameAnimation]")
{
    FrameAnimation frameAnimation;
    REQUIRE(frameAnimation.getCurrentFrame() == 0);

    frameAnimation.update(0.5f);
    REQUIRE(frameAnimation.getCurrentFrame() == 0);

    frameAnimation.reset();
    REQUIRE(frameAnimation.getCurrentFrame() == 0);
}

TEST_CASE("FrameAnimation updates frame based on time", "[FrameAnimation]")
{
    FrameAnimation frameAnimation({{1, 2, 3}, 0.5f});

    SECTION("Starts at first frame")
    {
        REQUIRE(frameAnimation.getCurrentFrame() == 1);
    }

    SECTION("Advances to next frame after time")
    {
        frameAnimation.update(0.5f);
        REQUIRE(frameAnimation.getCurrentFrame() == 2);
    }

    SECTION("Wraps around after all frames")
    {
        frameAnimation.update(1.5f);
        REQUIRE(frameAnimation.getCurrentFrame() == 1);
    }

    SECTION("Multiple small steps accumulate")
    {
        frameAnimation.update(0.2f);
        frameAnimation.update(0.2f);
        frameAnimation.update(0.2f);
        REQUIRE(frameAnimation.getCurrentFrame() == 2);
    }

    SECTION("Reset returns to frame 0")
    {
        frameAnimation.update(1.0f);
        frameAnimation.reset();
        REQUIRE(frameAnimation.getCurrentFrame() == 1);
    }
}