#include <catch2/catch_test_macros.hpp>
#include "game/actor/actor_motion_state.hpp"
#include "game/actor/abilities/climb_ability.hpp"
#include "input/input_intentions.hpp"

TEST_CASE("ClimbAbility basic movement behaviour", "[ClimbAbility]")
{
    ActorMotionState state;
    InputIntentions inputIntentions;
    ClimbAbilityData climbAbilityData;
    ClimbAbility climbAbility(climbAbilityData);

    SECTION("Can climb")
    {
        state.contacts.touchingLeftWall = true;
        inputIntentions.climbRequested = true;
        climbAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.climbing);
    }

    SECTION("Cannot climb without touching wall")
    {
        inputIntentions.climbRequested = true;
        climbAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.climbing);
    }
}