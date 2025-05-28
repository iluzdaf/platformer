#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "rendering/texture2d.hpp"

TEST_CASE("Texture2D is valid", "[Texture2D]")
{
    Texture2D texture("../assets/textures/player.png");
    REQUIRE(texture.valid());
}

TEST_CASE("Texture2D does not exist", "[Texture2D]")
{
    REQUIRE_THROWS_WITH(
        Texture2D("../assets/textures/does_not_exist.png"),
        "Failed to load Texture2D");
}