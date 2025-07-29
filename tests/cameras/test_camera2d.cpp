#include <catch2/catch_test_macros.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <catch2/catch_approx.hpp>
#include "cameras/camera2d.hpp"
#include "cameras/camera2d_data.hpp"
using Catch::Approx;

TEST_CASE("Camera2D clamps target to bounds", "[Camera2D]")
{
    Camera2D camera(Camera2DData(1.0f), 100, 80);
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

TEST_CASE("Camera2D resize updates projection", "[Camera2D]")
{
    Camera2DData data(1.0f);
    Camera2D camera(data, 800, 600);
    camera.resize(1024, 768);
    glm::mat4 resizedProjection = camera.getProjection();

    float halfW = 1024 / (2.0f * data.zoom);
    float halfH = 768 / (2.0f * data.zoom);
    glm::mat4 expected = glm::ortho(
        camera.getPosition().x - halfW,
        camera.getPosition().x + halfW,
        camera.getPosition().y + halfH,
        camera.getPosition().y - halfH);

    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            REQUIRE(resizedProjection[col][row] == Approx(expected[col][row]));
        }
    }
}