#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/wall_hang_ability.hpp"
#include "input/input_intentions.hpp"

TEST_CASE("WallHangAbility basic movement behaviour", "[WallHangAbility]")
{
    ActorMotionState state;
    InputIntentions inputIntentions;
    WallHangAbilityData wallHangAbilityData;
    WallHangAbility wallHangAbility(wallHangAbilityData);

    SECTION("Can climb")
    {
        state.contacts.touchingLeftWall = true;
        inputIntentions.climbRequested = true;
        wallHangAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallHang.active);
    }

    SECTION("Cannot climb without touching wall")
    {
        inputIntentions.climbRequested = true;
        wallHangAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallHang.active);
    }
}