#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "agent/movement_abilities/action_buffer.hpp"

TEST_CASE("ActionBuffer basic behavior", "[ActionBuffer]")
{
    ActionBuffer buffer(0.2f);

    SECTION("Initially not buffered")
    {
        REQUIRE_FALSE(buffer.isBuffered());
    }

    SECTION("Buffer is set on press()")
    {
        buffer.press();
        REQUIRE(buffer.isBuffered());
    }

    SECTION("Buffer times out after duration")
    {
        buffer.press();
        buffer.update(0.1f);
        REQUIRE(buffer.isBuffered());
        buffer.update(0.2f);
        REQUIRE_FALSE(buffer.isBuffered());
    }

    SECTION("Consume clears buffer")
    {
        buffer.press();
        REQUIRE(buffer.isBuffered());
        buffer.consume();
        REQUIRE_FALSE(buffer.isBuffered());
    }

    SECTION("Throws on invalid duration")
    {
        REQUIRE_THROWS_WITH(ActionBuffer(0.0f), "duration must be greater than 0");
    }
}