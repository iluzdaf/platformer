#include <catch2/catch_test_macros.hpp>
#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"
#include "game/player/player_state.hpp"
#include "test_helpers/mock_player.hpp"

TEST_CASE("WallSlideAbility basic movement behaviour", "[WallSlideAbility]")
{
    PlayerState playerState;
    MockPlayer movementContext;
    WallSlideAbilityData wallSlideAbilityData;
    WallSlideAbility slideAbility(wallSlideAbilityData);

    SECTION("No wallSliding when not touching walls")
    {
        movementContext.setVelocity(glm::vec2(0.0f, 300.0f));
        playerState.touchingLeftWall = false;
        playerState.touchingRightWall = false;
        slideAbility.fixedUpdate(movementContext, playerState, 0.1f);
        slideAbility.syncState(playerState);
        slideAbility.update(movementContext, playerState, 0.1f);
        REQUIRE_FALSE(playerState.wallSliding);
        REQUIRE(movementContext.getVelocity().y == 300.0f);
    }

    SECTION("WallSliding when touching left wall")
    {
        movementContext.setVelocity(glm::vec2(0.0f, 300.0f));
        playerState.touchingLeftWall = true;
        slideAbility.fixedUpdate(movementContext, playerState, 0.1f);
        slideAbility.syncState(playerState);
        slideAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(playerState.wallSliding);
        REQUIRE(movementContext.getVelocity().y < 300.0f);
    }

    SECTION("WallSliding when touching right wall")
    {
        movementContext.setVelocity(glm::vec2(0.0f, 300.0f));
        playerState.touchingRightWall = true;
        slideAbility.fixedUpdate(movementContext, playerState, 0.1f);
        slideAbility.syncState(playerState);
        slideAbility.update(movementContext, playerState, 0.1f);
        REQUIRE(playerState.wallSliding);
        REQUIRE(movementContext.getVelocity().y < 300.0f);
    }

    SECTION("No wallSliding when on ground")
    {
        movementContext.setVelocity(glm::vec2(0.0f, 300.0f));
        playerState.onGround = true;
        playerState.touchingRightWall = true;
        slideAbility.tryMoveRight(movementContext, playerState);
        slideAbility.fixedUpdate(movementContext, playerState, 0.1f);
        slideAbility.syncState(playerState);
        slideAbility.update(movementContext, playerState, 0.1f);
        REQUIRE_FALSE(playerState.wallSliding);
    }

    SECTION("No wallSliding when not falling")
    {
        movementContext.setVelocity(glm::vec2(0.0f, 0.0f));
        playerState.touchingLeftWall = true;
        slideAbility.fixedUpdate(movementContext, playerState, 0.1f);
        slideAbility.syncState(playerState);
        slideAbility.update(movementContext, playerState, 0.1f);
        REQUIRE_FALSE(playerState.wallSliding);
        REQUIRE(movementContext.getVelocity().y == 0.0f);
    }

    SECTION("No wallSliding when climbing")
    {
        movementContext.setVelocity(glm::vec2(0.0f, 300.0f));
        playerState.touchingLeftWall = true;
        playerState.climbing = true;
        slideAbility.fixedUpdate(movementContext, playerState, 0.1f);
        slideAbility.syncState(playerState);
        slideAbility.update(movementContext, playerState, 0.1f);
        REQUIRE_FALSE(playerState.wallSliding);
        REQUIRE(movementContext.getVelocity().y == 300.0f);
    }
}