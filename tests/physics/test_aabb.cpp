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

    SECTION("Bottom center sits on the lower edge")
    {
        glm::vec2 bottomCenter = b.bottomCenter();
        REQUIRE(bottomCenter.x == Approx(10.0f));
        REQUIRE(bottomCenter.y == Approx(15.0f));
    }
}

TEST_CASE("A box covers where it starts and not where it ends", "[AABB]")
{
    AABB box{glm::vec2(10.0f, 20.0f), glm::vec2(4.0f, 6.0f)};

    REQUIRE(box.covers(glm::vec2(10.0f, 20.0f)));
    REQUIRE(box.covers(glm::vec2(13.9f, 25.9f)));
    REQUIRE_FALSE(box.covers(glm::vec2(14.0f, 23.0f)));
    REQUIRE_FALSE(box.covers(glm::vec2(12.0f, 26.0f)));
    REQUIRE_FALSE(box.covers(glm::vec2(9.9f, 23.0f)));
    REQUIRE_FALSE(box.covers(glm::vec2(12.0f, 19.9f)));
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

    SECTION("Including an empty AABB changes nothing")
    {
        AABB a{glm::vec2(5, 5), glm::vec2(10, 10)};
        a.expandToInclude(AABB{});
        REQUIRE(a.position.x == Approx(5.0f));
        REQUIRE(a.position.y == Approx(5.0f));
        REQUIRE(a.size.x == Approx(10.0f));
        REQUIRE(a.size.y == Approx(10.0f));
    }
}