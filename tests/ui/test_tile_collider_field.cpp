#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/tile_collider_field.hpp"

namespace
{
    constexpr float CellSize = 16.0f;
    constexpr float Scale = ColliderPreviewSize / CellSize;
}

TEST_CASE("A collider covering the whole tile covers the whole preview", "[ColliderPreview]")
{
    auto [low, high] = colliderRect(ImVec2(0.0f, 0.0f), Scale, glm::vec2(0.0f), glm::vec2(16.0f));

    REQUIRE(low.x == 0.0f);
    REQUIRE(low.y == 0.0f);
    REQUIRE(high.x == ColliderPreviewSize);
    REQUIRE(high.y == ColliderPreviewSize);
}

TEST_CASE("A spike's collider lies along the bottom of its tile", "[ColliderPreview]")
{
    auto [low, high] =
        colliderRect(ImVec2(0.0f, 0.0f), Scale, glm::vec2(0.0f, 12.0f), glm::vec2(16.0f, 4.0f));

    REQUIRE(low.y == ColliderPreviewSize * 0.75f);
    REQUIRE(high.y == ColliderPreviewSize);
    REQUIRE(low.x == 0.0f);
    REQUIRE(high.x == ColliderPreviewSize);
}

TEST_CASE("A collider up the left side stays up the left side", "[ColliderPreview]")
{
    auto [low, high] =
        colliderRect(ImVec2(0.0f, 0.0f), Scale, glm::vec2(0.0f), glm::vec2(4.0f, 16.0f));

    REQUIRE(high.x == ColliderPreviewSize * 0.25f);
    REQUIRE(high.y == ColliderPreviewSize);
}

TEST_CASE("A collider is drawn from where the tile is", "[ColliderPreview]")
{
    auto [low, high] =
        colliderRect(ImVec2(30.0f, 70.0f), Scale, glm::vec2(4.0f, 4.0f), glm::vec2(8.0f));

    REQUIRE(low.x == 30.0f + 24.0f);
    REQUIRE(low.y == 70.0f + 24.0f);
    REQUIRE(high.x == low.x + 48.0f);
    REQUIRE(high.y == low.y + 48.0f);
}
