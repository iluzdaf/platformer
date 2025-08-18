#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "agent/movement_abilities/direction_buffer.hpp"

TEST_CASE("DirectionBuffer behaves correctly", "[DirectionBuffer]")
{
    DirectionBuffer buffer(0.2f);

    SECTION("Initial state is empty")
    {
        REQUIRE_FALSE(buffer.isBuffered());
        REQUIRE(buffer.getBufferedDirectionX() == Catch::Approx(0.0f));
    }

    SECTION("Press stores direction and activates buffer")
    {
        buffer.press(-1.0f);
        REQUIRE(buffer.isBuffered());
        REQUIRE(buffer.getBufferedDirectionX() == Catch::Approx(-1.0f));
    }

    SECTION("Update decreases buffer timer")
    {
        buffer.press(1.0f);
        buffer.update(0.1f);
        REQUIRE(buffer.isBuffered());
        buffer.update(0.2f);
        REQUIRE_FALSE(buffer.isBuffered());
    }

    SECTION("Consume clears buffer")
    {
        buffer.press(1.0f);
        buffer.consume();
        REQUIRE_FALSE(buffer.isBuffered());
        REQUIRE(buffer.getBufferedDirectionX() == Catch::Approx(0.0f));
    }
}