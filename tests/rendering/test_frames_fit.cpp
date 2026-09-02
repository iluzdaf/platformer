#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation_data.hpp"
#include "assets/sheet.hpp"
#include "rendering/frames_fit.hpp"

namespace
{
    constexpr int Wide = 160, Tall = 16;

    Sheet cells(int width, int height)
    {
        return Sheet{"textures/coin.png", glm::ivec2(width, height)};
    }
}

TEST_CASE("Frames inside the sheet fit it", "[FramesFit]")
{
    FrameAnimationData animation{{0, 5, 9}, 0.1f};

    REQUIRE_NOTHROW(checkFramesFit(animation, cells(16, 16), "\"coin\"", Wide, Tall));
}

TEST_CASE("An animation with no frames fits anything", "[FramesFit]")
{
    REQUIRE_NOTHROW(checkFramesFit(FrameAnimationData{}, cells(16, 16), "\"coin\"", Wide, Tall));
}

TEST_CASE("A frame past the end says which frame and how many there are", "[FramesFit]")
{
    FrameAnimationData animation{{0, 10}, 0.1f};

    REQUIRE_THROWS_WITH(
        checkFramesFit(animation, cells(16, 16), "\"coin\"", Wide, Tall),
        Catch::Matchers::ContainsSubstring("coin") &&
            Catch::Matchers::ContainsSubstring("frame 10") &&
            Catch::Matchers::ContainsSubstring("holds 10"));
}

TEST_CASE("A frame below the first is past the end too", "[FramesFit]")
{
    FrameAnimationData animation{{-1}, 0.1f};

    REQUIRE_THROWS(checkFramesFit(animation, cells(16, 16), "\"coin\"", Wide, Tall));
}

TEST_CASE("Cells no bigger than nothing hold no frames", "[FramesFit]")
{
    FrameAnimationData animation{{0}, 0.1f};

    REQUIRE_THROWS_WITH(
        checkFramesFit(animation, cells(0, 16), "\"coin\"", Wide, Tall),
        Catch::Matchers::ContainsSubstring("no wider or taller"));
}

TEST_CASE("Taller cells mean fewer of them in the same sheet", "[FramesFit]")
{
    FrameAnimationData animation{{5}, 0.1f};

    REQUIRE_NOTHROW(checkFramesFit(animation, cells(16, 16), "\"short\"", 32, 64));
    REQUIRE_THROWS(checkFramesFit(animation, cells(16, 32), "\"tall\"", 32, 64));
}
