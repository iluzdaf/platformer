#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/wall_hang_ability.hpp"
#include "input/input_intentions.hpp"

TEST_CASE("WallHangAbility basic movement behaviour", "[WallHangAbility]")
{
    Decided state;
    Observed observed;
    InputIntentions inputIntentions;
    WallHangAbilityData wallHangAbilityData;
    WallHangAbility wallHangAbility(wallHangAbilityData);

    SECTION("Can climb")
    {
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = true;
        inputIntentions.climbRequested = true;
        wallHangAbility.applyMovement(0.01f, inputIntentions, observed, state);
        REQUIRE(state.wallHang.active);
    }

    SECTION("Cannot climb without touching wall")
    {
        inputIntentions.climbRequested = true;
        wallHangAbility.applyMovement(0.01f, inputIntentions, observed, state);
        REQUIRE_FALSE(state.wallHang.active);
    }
}