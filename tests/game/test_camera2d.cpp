#include <catch2/catch_test_macros.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/camera2d.hpp"
#include <catch2/catch_approx.hpp>
using Catch::Approx;

TEST_CASE("Camera2D clamps target to bounds", "[Camera2D]")
{
    Camera2D camera(100.0f, 80.0f, 1.0f);
    camera.setWorldBounds(glm::vec2(0, 0), glm::vec2(200, 200));

    SECTION("Camera centers on target inside bounds")
    {
        camera.follow(glm::vec2(100.0f, 100.0f));
        glm::vec2 pos = camera.getPosition();
        REQUIRE(pos == glm::vec2(100.0f, 100.0f));
    }

    SECTION("Camera clamps to min bounds")
    {
        camera.follow(glm::vec2(-50.0f, -50.0f));
        glm::vec2 pos = camera.getPosition();
        REQUIRE(pos.x == Approx(50.0f));
        REQUIRE(pos.y == Approx(40.0f));
    }

    SECTION("Camera clamps to max bounds")
    {
        camera.follow(glm::vec2(9999.0f, 9999.0f));
        glm::vec2 pos = camera.getPosition();
        REQUIRE(pos.x == Approx(150.0f));
        REQUIRE(pos.y == Approx(160.0f));
    }
}