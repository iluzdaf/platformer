#include <catch2/catch_test_macros.hpp>
#include "texture2d.hpp"

TEST_CASE("Texture2D is valid", "[texture2d]")
{
    Texture2D texture("../textures/player.png");
    REQUIRE(texture.valid());
}

TEST_CASE("Texture2D does not exist", "[texture]")
{
    Texture2D texture("../textures/does_not_exist.png");
    REQUIRE_FALSE(texture.valid());
}