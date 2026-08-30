#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/abilities/wall_slide_ability.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "input/input_intentions.hpp"
#include "actor/actor_motion_state.hpp"

using Catch::Approx;

TEST_CASE("WallSlideAbility basic movement behaviour", "[WallSlideAbility]")
{
    ActorMotionState state;
    InputIntentions inputIntentions;
    WallSlideAbilityData wallSlideAbilityData;
    WallSlideAbility slideAbility(wallSlideAbilityData);

    SECTION("Can wall slide")
    {
        state.contacts.touchingLeftWall = state.contacts.grippableLeftWall = true;
        state.contacts.onGround = false;
        state.velocity.y = 980.0f;
        slideAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE(state.wallSlide.active);
        REQUIRE(state.wallSlide.velocity.y == Approx(wallSlideAbilityData.slideSpeed));
    }

    SECTION("Cannot wall slide if not touching wall")
    {
        state.contacts.onGround = false;
        slideAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallSlide.active);
        REQUIRE(state.wallSlide.velocity.y == Approx(0.0f));
    }

    SECTION("Cannot wall slide when on ground")
    {
        state.contacts.touchingLeftWall = state.contacts.grippableLeftWall = true;
        state.contacts.onGround = true;
        slideAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallSlide.active);
        REQUIRE(state.wallSlide.velocity.y == Approx(0.0f));
    }

    SECTION("Cannot wall slide if not falling")
    {
        state.contacts.touchingLeftWall = state.contacts.grippableLeftWall = true;
        state.contacts.onGround = false;
        state.velocity.y = 0.0f;
        slideAbility.applyMovement(0.01f, inputIntentions, state);
        REQUIRE_FALSE(state.wallSlide.active);
        REQUIRE(state.wallSlide.velocity.y == Approx(0.0f));
    }
}