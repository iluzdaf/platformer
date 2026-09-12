#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/wall_hang_ability.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/abilities/ability_states.hpp"
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
    AbilityStates states;

    tick(hang, askingToClimb(), onAWall(WallSide::Left), states);
    REQUIRE(states.wallHang.active);

    tick(hang, askingToClimb(), onAWall(WallSide::Right), states);
    REQUIRE(states.wallHang.active);
}

TEST_CASE("Asking to climb away from a wall hangs on nothing", "[WallHangAbility]")
{
    WallHangAbility hang(WallHangAbilityData{});
    AbilityStates states;

    tick(hang, askingToClimb(), inTheAir(), states);

    REQUIRE_FALSE(states.wallHang.active);
}

TEST_CASE("A wall it cannot grip is not hung on", "[WallHangAbility]")
{
    WallHangAbility hang(WallHangAbilityData{});
    AbilityStates states;

    tick(hang, askingToClimb(), onASlipperyWall(WallSide::Left), states);

    REQUIRE_FALSE(states.wallHang.active);
}

TEST_CASE("No longer asking to climb lets go of the wall", "[WallHangAbility]")
{
    WallHangAbility hang(WallHangAbilityData{});
    AbilityStates states;
    tick(hang, askingToClimb(), onAWall(WallSide::Left), states);

    tick(hang, InputIntentions{}, onAWall(WallSide::Left), states);

    REQUIRE_FALSE(states.wallHang.active);
}
