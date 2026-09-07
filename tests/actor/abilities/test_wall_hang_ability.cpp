#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/wall_hang_ability.hpp"
#include "input/input_intentions.hpp"

TEST_CASE("WallHangAbility basic movement behaviour", "[WallHangAbility]")
{
    Decided decided;
    Observed observed;
    InputIntentions inputIntentions;
    WallHangAbilityData wallHangAbilityData;
    WallHangAbility wallHangAbility(wallHangAbilityData);

    SECTION("Can climb")
    {
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = true;
        inputIntentions.climbRequested = true;
        wallHangAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallHang.active);
    }

    SECTION("Cannot climb without touching wall")
    {
        inputIntentions.climbRequested = true;
        wallHangAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallHang.active);
    }
}