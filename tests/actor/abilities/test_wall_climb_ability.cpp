#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/wall_climb_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("WallClimbAbility basic movement behaviour", "[WallClimbAbility]")
{
    Decided decided;
    Observed observed;
    InputIntentions inputIntentions;
    WallClimbAbilityData wallClimbAbilityData;
    WallClimbAbility wallClimbAbility(wallClimbAbilityData);

    SECTION("Can climb up")
    {
        decided.wallHang.active = true;
        inputIntentions.direction.y = -1;
        wallClimbAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallClimb.velocity.y == Approx(-wallClimbAbilityData.climbSpeed));
    }

    SECTION("Can climb down")
    {
        decided.wallHang.active = true;
        inputIntentions.direction.y = 1;
        wallClimbAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallClimb.velocity.y == Approx(wallClimbAbilityData.climbSpeed));
    }

    SECTION("Cannot climb up if not climbing")
    {
        inputIntentions.direction.y = -1;
        wallClimbAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallClimb.velocity.y == Approx(0.0f));
    }

    SECTION("Cannot climb down if not climbing")
    {
        inputIntentions.direction.y = 1;
        wallClimbAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallClimb.velocity.y == Approx(0.0f));
    }

    SECTION("If no direction requested, no movement applied")
    {
        decided.wallHang.active = true;
        wallClimbAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallClimb.velocity.y == Approx(0.0f));
    }
}

TEST_CASE("A climb that goes nowhere is refused", "[WallClimbAbility]")
{
    WallClimbAbilityData noSpeed;
    noSpeed.climbSpeed = 0.0f;
    REQUIRE_THROWS(WallClimbAbility(noSpeed));
}
