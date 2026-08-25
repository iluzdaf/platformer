#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game/actor/abilities/wall_slide_ability.hpp"
#include "input/input_intentions.hpp"
#include "game/actor/actor_motion_state.hpp"

using Catch::Approx;

TEST_CASE("WallSlideAbility basic movement behaviour", "[WallSlideAbility]")
{
    ActorMotionState state;
    InputIntentions inputIntentions;
    WallSlideAbilityData wallSlideAbilityData;
    WallSlideAbility slideAbility(wallSlideAbilityData);

    SECTION("Can wall slide")
    {
        state.touchingLeftWall = true;
        state.onGround = false;
        state.velocity.y = 980.0f;
        slideAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallSliding);
        REQUIRE(state.wallSlideVelocity.y == Approx(wallSlideAbilityData.slideSpeed));
    }

    SECTION("Cannot wall slide if not touching wall")
    {
        state.onGround = false;
        slideAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallSliding);
        REQUIRE(state.wallSlideVelocity.y == Approx(0.0f));
    }

    SECTION("Cannot wall slide when on ground")
    {
        state.touchingLeftWall = true;
        state.onGround = true;
        slideAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallSliding);
        REQUIRE(state.wallSlideVelocity.y == Approx(0.0f));
    }

    SECTION("Cannot wall slide if not falling")
    {
        state.touchingLeftWall = true;
        state.onGround = false;
        state.velocity.y = 0.0f;
        slideAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallSliding);
        REQUIRE(state.wallSlideVelocity.y == Approx(0.0f));
    }
}