#include <catch2/catch_all.hpp>
#include <catch2/catch_approx.hpp>
#include "physics/aabb.hpp"
using Catch::Approx;

TEST_CASE("AABB intersection and center", "[AABB]")
{
    AABB a{glm::vec2(0, 0), glm::vec2(10, 10)};
    AABB b{glm::vec2(5, 5), glm::vec2(10, 10)};
    AABB c{glm::vec2(20, 20), glm::vec2(5, 5)};

    SECTION("Intersects overlapping AABB")
    {
        REQUIRE(a.intersects(b));
    }

    SECTION("Does not intersect non-overlapping AABB")
    {
        REQUIRE_FALSE(a.intersects(c));
    }

    SECTION("Center is correctly computed")
    {
        glm::vec2 center = a.center();
        REQUIRE(center.x == Approx(5.0f));
        REQUIRE(center.y == Approx(5.0f));
    }
}

TEST_CASE("AABB expandToInclude", "[AABB]")
{
    SECTION("Expands to include overlapping AABB")
    {
        AABB a{glm::vec2(0, 0), glm::vec2(10, 10)};
        AABB b{glm::vec2(5, 5), glm::vec2(10, 10)};
        a.expandToInclude(b);
        REQUIRE(a.position.x == Approx(0.0f));
        REQUIRE(a.position.y == Approx(0.0f));
        REQUIRE(a.size.x == Approx(15.0f));
        REQUIRE(a.size.y == Approx(15.0f));
    }

    SECTION("Expands to include non-overlapping AABB")
    {
        AABB a{glm::vec2(0, 0), glm::vec2(10, 10)};
        AABB c{glm::vec2(20, 20), glm::vec2(5, 5)};
        a.expandToInclude(c);
        REQUIRE(a.position.x == Approx(0.0f));
        REQUIRE(a.position.y == Approx(0.0f));
        REQUIRE(a.size.x == Approx(25.0f));
        REQUIRE(a.size.y == Approx(25.0f));
    }

    SECTION("Expands to include entirely smaller AABB")
    {
        AABB a{glm::vec2(5, 5), glm::vec2(10, 10)};
        AABB d{glm::vec2(7, 7), glm::vec2(2, 2)};
        a.expandToInclude(d);
        REQUIRE(a.position.x == Approx(5.0f));
        REQUIRE(a.position.y == Approx(5.0f));
        REQUIRE(a.size.x == Approx(10.0f));
        REQUIRE(a.size.y == Approx(10.0f));
    }
}