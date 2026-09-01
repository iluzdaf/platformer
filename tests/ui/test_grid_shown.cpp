#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "ui/grid_shown.hpp"

TEST_CASE("A grid nobody asked for is shown while something is armed", "[GridShown]")
{
    GridShown grid = whileArmed(GridShown{false, std::nullopt}, true);

    REQUIRE(grid.showing);
}

TEST_CASE("A grid shown only for arming goes away again", "[GridShown]")
{
    GridShown grid = whileArmed(GridShown{false, std::nullopt}, true);

    REQUIRE(whileArmed(grid, false) == GridShown{false, std::nullopt});
}

TEST_CASE("A grid asked for stays after arming ends", "[GridShown]")
{
    GridShown grid = whileArmed(GridShown{true, std::nullopt}, true);

    REQUIRE(grid.showing);
    REQUIRE(whileArmed(grid, false) == GridShown{true, std::nullopt});
}

TEST_CASE("Staying armed remembers only what was there first", "[GridShown]")
{
    GridShown grid = whileArmed(GridShown{false, std::nullopt}, true);
    grid.showing = false;
    grid = whileArmed(grid, true);
    grid = whileArmed(grid, true);

    REQUIRE(grid.beforeArming == false);
    REQUIRE(whileArmed(grid, false) == GridShown{false, std::nullopt});
}

TEST_CASE("Staying disarmed leaves the grid alone", "[GridShown]")
{
    REQUIRE(whileArmed(GridShown{false, std::nullopt}, false) == GridShown{false, std::nullopt});
    REQUIRE(whileArmed(GridShown{true, std::nullopt}, false) == GridShown{true, std::nullopt});
}
