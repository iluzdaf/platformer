#include <catch2/catch_test_macros.hpp>
#include "animations/tile_animation.hpp"

TEST_CASE("TileAnimation advances frames over time", "[TileAnimation]")
{
    std::vector<int> frames = {10, 11, 12, 13};
    TileAnimation anim(frames, 0.25f);

    SECTION("Starts at first frame")
    {
        REQUIRE(anim.getCurrentFrame() == 10);
    }

    SECTION("Advances to next frame")
    {
        anim.update(0.25f);
        REQUIRE(anim.getCurrentFrame() == 11);
    }

    SECTION("Wraps around after all frames")
    {
        anim.update(1.0f);
        REQUIRE(anim.getCurrentFrame() == 10);
    }

    SECTION("Handles partial steps correctly")
    {
        anim.update(0.5f);
        REQUIRE(anim.getCurrentFrame() == 12);
    }

    SECTION("Ignores update when no time passed")
    {
        anim.update(0.0f);
        REQUIRE(anim.getCurrentFrame() == 10);
    }
}