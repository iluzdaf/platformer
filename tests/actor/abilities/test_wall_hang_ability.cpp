#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/wall_hang_ability.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/decided.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

namespace
{
    InputIntentions askingToClimb()
    {
        InputIntentions intentions;
        intentions.climbRequested = true;
        return intentions;
    }
}

TEST_CASE("Asking to climb against a wall it grips hangs on it", "[WallHangAbility]")
{
    WallHangAbility hang(WallHangAbilityData{});
    Decided decided;

    tick(hang, askingToClimb(), onAWall(WallSide::Left), decided);
    REQUIRE(decided.wallHang.active);

    tick(hang, askingToClimb(), onAWall(WallSide::Right), decided);
    REQUIRE(decided.wallHang.active);
}

TEST_CASE("Asking to climb away from a wall hangs on nothing", "[WallHangAbility]")
{
    WallHangAbility hang(WallHangAbilityData{});
    Decided decided;

    tick(hang, askingToClimb(), inTheAir(), decided);

    REQUIRE_FALSE(decided.wallHang.active);
}

TEST_CASE("A wall it cannot grip is not hung on", "[WallHangAbility]")
{
    WallHangAbility hang(WallHangAbilityData{});
    Decided decided;

    tick(hang, askingToClimb(), onASlipperyWall(WallSide::Left), decided);

    REQUIRE_FALSE(decided.wallHang.active);
}

TEST_CASE("No longer asking to climb lets go of the wall", "[WallHangAbility]")
{
    WallHangAbility hang(WallHangAbilityData{});
    Decided decided;
    tick(hang, askingToClimb(), onAWall(WallSide::Left), decided);

    tick(hang, InputIntentions{}, onAWall(WallSide::Left), decided);

    REQUIRE_FALSE(decided.wallHang.active);
}
