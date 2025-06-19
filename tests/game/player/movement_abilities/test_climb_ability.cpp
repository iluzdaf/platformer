#include <catch2/catch_test_macros.hpp>
#include "game/player/movement_abilities/climb_ability.hpp"
#include "game/player/movement_abilities/climb_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"

TEST_CASE("ClimbAbility basic movement behaviour", "[ClimbAbility]")
{
    PlayerState playerState;
    MockPlayer movementContext;
    ClimbAbilityData climbAbilityData;
    ClimbAbility climbAbility(climbAbilityData);

    SECTION("Can climb")
    {
        playerState.touchingLeftWall = true;
        climbAbility.tryClimb(movementContext, playerState);
        climbAbility.fixedUpdate(movementContext, playerState, 0.01f);
        climbAbility.syncState(playerState);
        climbAbility.update(movementContext, playerState, 0.01f);
        REQUIRE(playerState.climbing);
    }

    SECTION("Cannot climb without touching wall")
    {
        climbAbility.tryClimb(movementContext, playerState);
        climbAbility.fixedUpdate(movementContext, playerState, 0.01f);
        climbAbility.syncState(playerState);
        climbAbility.update(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.climbing);
    }

    SECTION("Cannot climb longer than climb duration")
    {
        playerState.touchingLeftWall = true;
        climbAbility.tryClimb(movementContext, playerState);
        float deltaTime = climbAbility.getClimbDuration() + 0.01f;
        climbAbility.fixedUpdate(movementContext, playerState, deltaTime);
        climbAbility.syncState(playerState);
        climbAbility.update(movementContext, playerState, deltaTime);
        REQUIRE_FALSE(playerState.climbing);
    }

    SECTION("Climb time resets on ground")
    {
        playerState.touchingLeftWall = true;
        climbAbility.tryClimb(movementContext, playerState);
        float deltaTime = climbAbility.getClimbDuration() + 0.01f;
        climbAbility.fixedUpdate(movementContext, playerState, deltaTime);
        climbAbility.update(movementContext, playerState, deltaTime);
        playerState.onGround = true;
        climbAbility.fixedUpdate(movementContext, playerState, 0.01f);
        climbAbility.update(movementContext, playerState, 0.01f);
        climbAbility.tryClimb(movementContext, playerState);
        climbAbility.fixedUpdate(movementContext, playerState, 0.01f);
        climbAbility.syncState(playerState);
        climbAbility.update(movementContext, playerState, 0.01f);
        REQUIRE(playerState.climbing);
    }

    SECTION("Cannot climb when wall jumping")
    {
        playerState.touchingLeftWall = true;
        playerState.wallJumping = true;
        climbAbility.tryClimb(movementContext, playerState);
        climbAbility.fixedUpdate(movementContext, playerState, 0.01f);
        climbAbility.syncState(playerState);
        climbAbility.update(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.climbing);
    }
}