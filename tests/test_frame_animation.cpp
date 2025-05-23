#include <catch2/catch_test_macros.hpp>
#include "game/frame_animation.hpp"

TEST_CASE("FrameAnimation updates frame based on time", "[FrameAnimation]")
{
    FrameAnimation anim({1, 2, 3}, 0.5f);

    SECTION("Starts at first frame")
    {
        REQUIRE(anim.getCurrentFrame() == 1);
    }

    SECTION("Advances to next frame after time")
    {
        anim.update(0.5f);
        REQUIRE(anim.getCurrentFrame() == 2);
    }

    SECTION("Wraps around after all frames")
    {
        anim.update(1.5f);
        REQUIRE(anim.getCurrentFrame() == 1);
    }

    SECTION("Multiple small steps accumulate")
    {
        anim.update(0.2f);
        anim.update(0.2f);
        anim.update(0.2f);
        REQUIRE(anim.getCurrentFrame() == 2);
    }

    SECTION("Reset returns to frame 0")
    {
        anim.update(1.0f);
        anim.reset();
        REQUIRE(anim.getCurrentFrame() == 1);
    }
}