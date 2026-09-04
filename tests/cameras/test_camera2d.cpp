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

TEST_CASE("A world smaller than the view is centred in it", "[Camera2D]")
{
    Camera2D camera(Camera2DData(1.0f), 100, 80);
    camera.setWorldBounds(glm::vec2(0, 0), glm::vec2(50, 50));

    camera.follow(glm::vec2(9999.0f, -9999.0f));

    REQUIRE(camera.getPosition() == glm::vec2(25.0f, 25.0f));
}

TEST_CASE("Bounds that do not span anything are refused", "[Camera2D]")
{
    Camera2D camera(Camera2DData(1.0f), 100, 80);

    REQUIRE_THROWS(camera.setWorldBounds(glm::vec2(10, 0), glm::vec2(10, 100)));
    REQUIRE_THROWS(camera.setWorldBounds(glm::vec2(0, 10), glm::vec2(100, 10)));
}

TEST_CASE("A window with no size is refused", "[Camera2D]")
{
    Camera2D camera(Camera2DData(1.0f), 100, 80);

    REQUIRE_THROWS(camera.resize(0, 80));
    REQUIRE_THROWS(camera.resize(100, 0));
}

TEST_CASE("A shake moves the view for as long as it was asked to", "[Camera2D]")
{
    Camera2D camera(Camera2DData(1.0f), 100, 80);
    glm::mat4 still = camera.getProjection();

    camera.startShake(0.2f, 8.0f);
    REQUIRE(camera.shaking());

    bool moved = false;
    for (int step = 0; step < 30; ++step)
    {
        camera.update(0.01f);
        moved = moved || camera.getProjection() != still;
    }

    REQUIRE(moved);
    REQUIRE_FALSE(camera.shaking());
}

TEST_CASE("A shake of no time or no size is refused", "[Camera2D]")
{
    Camera2D camera(Camera2DData(1.0f), 100, 80);

    REQUIRE_THROWS(camera.startShake(0.0f, 8.0f));
    REQUIRE_THROWS(camera.startShake(0.2f, 0.0f));
}

TEST_CASE("Zoom is what it was set to, and must be positive", "[Camera2D]")
{
    Camera2D camera(Camera2DData(1.0f), 100, 80);

    camera.setZoom(2.0f);

    REQUIRE(camera.getZoom() == 2.0f);
    REQUIRE_THROWS(camera.setZoom(0.0f));
}

TEST_CASE("The view's top left is half a window up and left of the camera", "[Camera2D]")
{
    Camera2D camera(Camera2DData(1.0f), 100, 80);
    camera.setWorldBounds(glm::vec2(0, 0), glm::vec2(1000, 1000));
    camera.follow(glm::vec2(100.0f, 100.0f));

    REQUIRE(camera.getWindowSize() == glm::vec2(100.0f, 80.0f));
    REQUIRE(camera.getTopLeftPosition() == glm::vec2(50.0f, 60.0f));

    camera.setZoom(2.0f);

    REQUIRE(camera.getTopLeftPosition() == glm::vec2(75.0f, 80.0f));
}
