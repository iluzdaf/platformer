#include <catch2/catch_all.hpp>
#include <catch2/catch_approx.hpp>
#include "physics/aabb.hpp"
using Catch::Approx;

TEST_CASE("AABB intersection and center", "[AABB]") {
    AABB a{glm::vec2(0, 0), glm::vec2(10, 10)};
    AABB b{glm::vec2(5, 5), glm::vec2(10, 10)};
    AABB c{glm::vec2(20, 20), glm::vec2(5, 5)};

    SECTION("Intersects overlapping AABB") {
        REQUIRE(a.intersects(b));
    }

    SECTION("Does not intersect non-overlapping AABB") {
        REQUIRE_FALSE(a.intersects(c));
    }

    SECTION("Center is correctly computed") {
        glm::vec2 center = a.center();
        REQUIRE(center.x == Approx(5.0f));
        REQUIRE(center.y == Approx(5.0f));
    }
}