#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/wall_climb_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("WallClimbAbility basic movement behaviour", "[WallClimbAbility]")
{
    ActorMotionState state;
    InputIntentions inputIntentions;
    WallClimbAbilityData wallClimbAbilityData;
    WallClimbAbility wallClimbAbility(wallClimbAbilityData);

    SECTION("Can climb up")
    {
        state.wallHang.active = true;
        inputIntentions.direction.y = -1;
        wallClimbAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallClimb.velocity.y == Approx(-wallClimbAbilityData.climbSpeed));
    }

    SECTION("Can climb down")
    {
        state.wallHang.active = true;
        inputIntentions.direction.y = 1;
        wallClimbAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallClimb.velocity.y == Approx(wallClimbAbilityData.climbSpeed));
    }

    SECTION("Cannot climb up if not climbing")
    {
        inputIntentions.direction.y = -1;
        wallClimbAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallClimb.velocity.y == Approx(0.0f));
    }

    SECTION("Cannot climb down if not climbing")
    {
        inputIntentions.direction.y = 1;
        wallClimbAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallClimb.velocity.y == Approx(0.0f));
    }

    SECTION("If no direction requested, no movement applied")
    {
        state.wallHang.active = true;
        wallClimbAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallClimb.velocity.y == Approx(0.0f));
    }
}