#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/abilities/wall_slide_ability.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "input/input_intentions.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"

using Catch::Approx;

TEST_CASE("WallSlideAbility basic movement behaviour", "[WallSlideAbility]")
{
    Decided decided;
    Observed observed;
    InputIntentions inputIntentions;
    WallSlideAbilityData wallSlideAbilityData;
    WallSlideAbility slideAbility(wallSlideAbilityData);

    SECTION("Can wall slide")
    {
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = true;
        observed.contacts.onGround = false;
        observed.velocity.y = 980.0f;
        slideAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.wallSlide.active);
        REQUIRE(decided.wallSlide.velocity.y == Approx(wallSlideAbilityData.slideSpeed));
    }

    SECTION("Cannot wall slide if not touching wall")
    {
        observed.contacts.onGround = false;
        slideAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallSlide.active);
        REQUIRE(decided.wallSlide.velocity.y == Approx(0.0f));
    }

    SECTION("Cannot wall slide when on ground")
    {
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = true;
        observed.contacts.onGround = true;
        slideAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallSlide.active);
        REQUIRE(decided.wallSlide.velocity.y == Approx(0.0f));
    }

    SECTION("Cannot wall slide if not falling")
    {
        observed.contacts.touchingLeftWall = observed.contacts.grippableLeftWall = true;
        observed.contacts.onGround = false;
        observed.velocity.y = 0.0f;
        slideAbility.applyMovement(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.wallSlide.active);
        REQUIRE(decided.wallSlide.velocity.y == Approx(0.0f));
    }
}

TEST_CASE("A slide that does not slide is refused", "[WallSlideAbility]")
{
    WallSlideAbilityData noSpeed;
    noSpeed.slideSpeed = 0.0f;
    REQUIRE_THROWS(WallSlideAbility(noSpeed));
}
