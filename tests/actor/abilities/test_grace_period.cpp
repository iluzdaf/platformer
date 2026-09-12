#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/grace_period.hpp"

TEST_CASE("A grace period is not running until it is started", "[GracePeriod]")
{
    GracePeriod grace(0.2f);

    REQUIRE_FALSE(grace.running());

    grace.start();
    REQUIRE(grace.running());
}

TEST_CASE("A grace period runs for as long as it lasts, and then stops", "[GracePeriod]")
{
    GracePeriod grace(0.2f);
    grace.start();

    grace.update(0.15f);
    REQUIRE(grace.running());

    grace.update(0.1f);
    REQUIRE_FALSE(grace.running());
}

TEST_CASE("Starting a grace period again gives it its whole length back", "[GracePeriod]")
{
    GracePeriod grace(0.2f);
    grace.start();
    grace.update(0.15f);

    grace.start();
    grace.update(0.15f);

    REQUIRE(grace.running());
}

TEST_CASE(
    "A grace period started every tick runs until the ticks stop starting it",
    "[GracePeriod]")
{
    GracePeriod grace(0.2f);
    for (int tick = 0; tick < 50; ++tick)
    {
        grace.update(0.1f);
        grace.start();
    }
    REQUIRE(grace.running());

    grace.update(0.15f);
    REQUIRE(grace.running());

    grace.update(0.1f);
    REQUIRE_FALSE(grace.running());
}

TEST_CASE("A grace period that is used stops at once", "[GracePeriod]")
{
    GracePeriod grace(0.2f);
    grace.start();

    grace.consume();

    REQUIRE_FALSE(grace.running());
}

TEST_CASE(
    "A grace period remembers the direction it was started with, until it stops",
    "[GracePeriod]")
{
    GracePeriod grace(0.2f);

    grace.start(-1.0f);
    REQUIRE(grace.direction() == -1.0f);

    grace.start(1.0f);
    REQUIRE(grace.direction() == 1.0f);

    grace.update(0.25f);
    REQUIRE(grace.direction() == 0.0f);

    grace.start(-1.0f);
    grace.consume();
    REQUIRE(grace.direction() == 0.0f);
}

TEST_CASE("A grace period started with no direction has none", "[GracePeriod]")
{
    GracePeriod grace(0.2f);

    grace.start();

    REQUIRE(grace.direction() == 0.0f);
}

TEST_CASE("A grace period with no length is refused, saying whose it is", "[GracePeriod]")
{
    REQUIRE_THROWS_WITH(
        GracePeriod(0.0f),
        Catch::Matchers::ContainsSubstring("A grace period needs a length above 0"));
    REQUIRE_THROWS_WITH(
        GracePeriod(-0.1f, "A roll's buffer"),
        Catch::Matchers::ContainsSubstring("A roll's buffer needs a length above 0"));
}
