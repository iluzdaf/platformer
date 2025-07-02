#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/coyote_time.hpp"

using Catch::Approx;

TEST_CASE("CoyoteTime basic behavior", "[CoyoteTime]")
{
    CoyoteTime coyote(0.2f);

    SECTION("Available immediately after update(true)")
    {
        coyote.update(true, 0.01f);
        REQUIRE(coyote.isCoyoteAvailable());
    }

    SECTION("Times out after duration")
    {
        coyote.update(true, 0.01f);
        coyote.update(false, 0.1f);
        coyote.update(false, 0.1f);
        REQUIRE_FALSE(coyote.isCoyoteAvailable());
    }

    SECTION("Consumes correctly")
    {
        coyote.update(true, 0.01f);
        REQUIRE(coyote.isCoyoteAvailable());
        coyote.consume();
        REQUIRE_FALSE(coyote.isCoyoteAvailable());
    }

    SECTION("Set duration updates duration")
    {
        coyote.setDuration(0.5f);
        REQUIRE(coyote.coyoteDuration == Approx(0.5f));
    }
}