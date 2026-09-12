#include <catch2/catch_test_macros.hpp>
#include "actor/facing.hpp"

TEST_CASE("An actor faces the way it moves", "[Facing]")
{
    REQUIRE(facingLeftAfter(false, -60.0f, false));
    REQUIRE_FALSE(facingLeftAfter(true, 60.0f, false));
}

TEST_CASE("An actor standing still keeps the way it faced", "[Facing]")
{
    REQUIRE(facingLeftAfter(true, 0.0f, false));
    REQUIRE_FALSE(facingLeftAfter(false, 0.0f, false));
}

TEST_CASE("An actor knocked back keeps the way it faced, whichever way it is pushed", "[Facing]")
{
    REQUIRE(facingLeftAfter(true, 200.0f, true));
    REQUIRE_FALSE(facingLeftAfter(false, -200.0f, true));
}
