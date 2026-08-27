#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/direction_buffer.hpp"

TEST_CASE("DirectionBuffer behaves correctly", "[DirectionBuffer]")
{
    DirectionBuffer directionBuffer;

    SECTION("Initial state is empty")
    {
        REQUIRE(directionBuffer.getBufferedDirectionX() == 0.0f);
    }

    SECTION("Press stores direction")
    {
        directionBuffer.press(-1.0f);
        REQUIRE(directionBuffer.getBufferedDirectionX() == -1.0f);

        directionBuffer.press(1.0f);
        REQUIRE(directionBuffer.getBufferedDirectionX() == 1.0f);
    }

    SECTION("Consume clears the stored direction")
    {
        directionBuffer.press(-1.0f);
        directionBuffer.consume();
        REQUIRE(directionBuffer.getBufferedDirectionX() == 0.0f);
    }
}
