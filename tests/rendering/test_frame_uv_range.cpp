#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/texture2d.hpp"

using Catch::Approx;

TEST_CASE("A frame reads the cell of the sheet its index names", "[FrameUvRange]")
{
    auto [start, end] = frameUvRangeIn(96, 96, 4, 32, 32);

    REQUIRE(start.x == Approx(1.0f / 3.0f));
    REQUIRE(start.y == Approx(1.0f / 3.0f));
    REQUIRE(end.x == Approx(2.0f / 3.0f));
    REQUIRE(end.y == Approx(2.0f / 3.0f));
}

TEST_CASE("Each axis of a sheet is divided by its own size", "[FrameUvRange]")
{
    auto [start, end] = frameUvRangeIn(96, 192, 3, 32, 32);

    REQUIRE(start.x == Approx(0.0f));
    REQUIRE(start.y == Approx(32.0f / 192.0f));
    REQUIRE(end.y == Approx(64.0f / 192.0f));
}

TEST_CASE("A frame taller than it is wide spans one whole cell", "[FrameUvRange]")
{
    auto [start, end] = frameUvRangeIn(96, 128, 0, 24, 64);

    REQUIRE(end.x - start.x == Approx(24.0f / 96.0f));
    REQUIRE(end.y - start.y == Approx(64.0f / 128.0f));
}

TEST_CASE("A sheet no frame fits in reads as the whole sheet", "[FrameUvRange]")
{
    auto [start, end] = frameUvRangeIn(96, 96, 0, 0, 32);

    REQUIRE(start == glm::vec2(0.0f));
    REQUIRE(end == glm::vec2(1.0f));
}
