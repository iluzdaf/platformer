#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/tile_map_overlays.hpp"

namespace
{
    constexpr int TileSize = 16;

    float lineAt(const TileGridOnScreen &grid, int step)
    {
        return grid.firstLine.x + static_cast<float>(step) * grid.spacing.x;
    }

    float whereTileBoundaryLands(float worldX, glm::vec2 cameraTopLeft, float zoom, float uiScale)
    {
        return ((worldX - cameraTopLeft.x) * zoom) / uiScale;
    }
}

TEST_CASE("Every grid line lands on a tile boundary", "[TileMapOverlays]")
{
    glm::vec2 cameraTopLeft(37.0f, 91.0f);
    float zoom = 4.5f;
    glm::vec2 uiScale(2.0f);

    TileGridOnScreen grid = tileGridOnScreen(cameraTopLeft, zoom, uiScale, TileSize);

    float firstBoundary = std::floor(cameraTopLeft.x / TileSize) * TileSize;
    for (int step = 0; step < 12; ++step)
    {
        float boundary = firstBoundary + static_cast<float>(step * TileSize);
        REQUIRE_THAT(
            lineAt(grid, step),
            Catch::Matchers::WithinAbs(
                whereTileBoundaryLands(boundary, cameraTopLeft, zoom, uiScale.x), 0.001f));
    }
}

TEST_CASE("The grid starts at the boundary above and left of the camera", "[TileMapOverlays]")
{
    glm::vec2 cameraTopLeft(37.0f, 91.0f);

    TileGridOnScreen grid = tileGridOnScreen(cameraTopLeft, 1.0f, glm::vec2(1.0f), TileSize);

    REQUIRE(grid.firstLine.x <= 0.0f);
    REQUIRE(grid.firstLine.y <= 0.0f);
    REQUIRE(grid.firstLine.x > -grid.spacing.x);
    REQUIRE(grid.firstLine.y > -grid.spacing.y);
}

TEST_CASE("The grid names the tile its first line belongs to", "[TileMapOverlays]")
{
    TileGridOnScreen grid =
        tileGridOnScreen(glm::vec2(37.0f, 91.0f), 1.0f, glm::vec2(1.0f), TileSize);

    REQUIRE(grid.topLeftTilePosition == glm::ivec2(2, 5));
}

TEST_CASE("A camera already on a boundary starts the grid there", "[TileMapOverlays]")
{
    TileGridOnScreen grid =
        tileGridOnScreen(glm::vec2(32.0f, 96.0f), 3.0f, glm::vec2(2.0f), TileSize);

    REQUIRE(grid.firstLine.x == 0.0f);
    REQUIRE(grid.firstLine.y == 0.0f);
    REQUIRE(grid.topLeftTilePosition == glm::ivec2(2, 6));
}

TEST_CASE("Grid lines spread apart as the camera zooms in", "[TileMapOverlays]")
{
    glm::vec2 cameraTopLeft(37.0f, 91.0f);

    TileGridOnScreen near = tileGridOnScreen(cameraTopLeft, 2.0f, glm::vec2(1.0f), TileSize);
    TileGridOnScreen far = tileGridOnScreen(cameraTopLeft, 4.0f, glm::vec2(1.0f), TileSize);

    REQUIRE(far.spacing.x == near.spacing.x * 2.0f);
    REQUIRE(far.spacing.y == near.spacing.y * 2.0f);
}

TEST_CASE("Each axis of the grid is scaled by its own axis", "[TileMapOverlays]")
{
    TileGridOnScreen grid =
        tileGridOnScreen(glm::vec2(0.0f), 1.0f, glm::vec2(2.0f, 4.0f), TileSize);

    REQUIRE(grid.spacing.x == TileSize / 2.0f);
    REQUIRE(grid.spacing.y == TileSize / 4.0f);
}
