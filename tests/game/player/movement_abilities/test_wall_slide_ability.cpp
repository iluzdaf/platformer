#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/player/movement_abilities/wall_slide_ability.hpp"
#include "game/player/movement_abilities/movement_context.hpp"
#include "game/player/player_state.hpp"

using Catch::Approx;

TEST_CASE("WallSlideAbility basic movement behaviour", "[WallSlideAbility]")
{
    PlayerState playerState;
    MovementContext movementContext;
    WallSlideAbilityData wallSlideAbilityData;
    WallSlideAbility slideAbility(wallSlideAbilityData);

    SECTION("Can wall slide")
    {
        playerState.touchingLeftWall = true;
        playerState.onGround = false;
        playerState.velocity.y = 980.0f;
        slideAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE(playerState.wallSliding);
        REQUIRE(movementContext.wallSlideVelocity.y == Approx(wallSlideAbilityData.slideSpeed));
    }

    SECTION("Cannot wall slide if not touching wall")
    {
        playerState.onGround = false;
        slideAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.wallSliding);
        REQUIRE(movementContext.wallSlideVelocity.y == Approx(0.0f));
    }

    SECTION("Cannot wall slide when on ground")
    {
        playerState.touchingLeftWall = true;
        playerState.onGround = true;
        slideAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.wallSliding);
        REQUIRE(movementContext.wallSlideVelocity.y == Approx(0.0f));
    }

    SECTION("Cannot wall slide if not falling")
    {
        playerState.touchingLeftWall = true;
        playerState.onGround = false;
        playerState.velocity.y = 0.0f;
        slideAbility.fixedUpdate(movementContext, playerState, 0.01f);
        REQUIRE_FALSE(playerState.wallSliding);
        REQUIRE(movementContext.wallSlideVelocity.y == Approx(0.0f));
    }
}