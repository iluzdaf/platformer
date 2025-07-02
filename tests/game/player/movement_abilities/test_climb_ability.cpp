#include <catch2/catch_test_macros.hpp>
#include "game/player/movement_abilities/climb_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

TEST_CASE("ClimbAbility basic movement behaviour", "[ClimbAbility]")
{
    PlayerState playerState;
    MovementContext movementContext;
    ClimbAbilityData climbAbilityData;
    ClimbAbility climbAbility(climbAbilityData);

    SECTION("Can climb")
    {
        playerState.touchingLeftWall = true;
        movementContext.inputIntentions.climbRequested = true;
        climbAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(playerState.climbing);
    }

    SECTION("Cannot climb without touching wall")
    {
        movementContext.inputIntentions.climbRequested = true;
        climbAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.climbing);
    }
}