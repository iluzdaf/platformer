#include <catch2/catch_test_macros.hpp>
#include "agent/agent_state.hpp"
#include "agent/movement_abilities/climb_ability.hpp"
#include "input/input_intentions.hpp"

TEST_CASE("ClimbAbility basic movement behaviour", "[ClimbAbility]")
{
    AgentState state;
    InputIntentions inputIntentions;
    ClimbAbilityData climbAbilityData;
    ClimbAbility climbAbility(climbAbilityData);

    SECTION("Can climb")
    {
        state.touchingLeftWall = true;
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