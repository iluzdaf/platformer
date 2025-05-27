#include <catch2/catch_test_macros.hpp>
#include "rendering/texture2d.hpp"

TEST_CASE("Texture2D is valid", "[Texture2D]")
{
    Texture2D texture("../assets/textures/player.png");
    REQUIRE(texture.valid());
}

TEST_CASE("Texture2D does not exist", "[Texture2D]")
{
    Texture2D texture("../assets/textures/does_not_exist.png");
    REQUIRE_FALSE(texture.valid());
}