#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/wall_slide_ability.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    Observed movingDown(Observed observed, float speed = 100.0f)
    {
        observed.velocity.y = speed;
        return observed;
    }
}

TEST_CASE("A slide goes down a wall it grips at its speed, while falling", "[WallSlideAbility]")
{
    WallSlideAbilityData data;
    WallSlideAbility slide(data);
    AbilityStates states;

    tick(slide, InputIntentions{}, movingDown(onAWall(WallSide::Left)), states);
    REQUIRE(states.wallSlide.active);
    REQUIRE(states.wallSlide.velocity.y == Approx(data.slideSpeed));

    tick(slide, InputIntentions{}, movingDown(onAWall(WallSide::Right)), states);
    REQUIRE(states.wallSlide.active);
}

TEST_CASE("A slide says it is sliding every tick it slides", "[WallSlideAbility]")
{
    WallSlideAbility slide(WallSlideAbilityData{});
    AbilityStates states;

    tick(slide, InputIntentions{}, movingDown(onAWall(WallSide::Left)), states);
    REQUIRE(states.wallSlide.emit);

    tick(slide, InputIntentions{}, movingDown(onAWall(WallSide::Left)), states);
    REQUIRE(states.wallSlide.emit);

    tick(slide, InputIntentions{}, movingDown(inTheAir()), states);
    REQUIRE_FALSE(states.wallSlide.emit);
}

TEST_CASE("A wall it cannot grip is not slid down", "[WallSlideAbility]")
{
    WallSlideAbility slide(WallSlideAbilityData{});
    AbilityStates states;

    tick(slide, InputIntentions{}, movingDown(onASlipperyWall(WallSide::Left)), states);

    REQUIRE_FALSE(states.wallSlide.active);
    REQUIRE(states.wallSlide.velocity.y == 0.0f);
}

TEST_CASE("Nothing is slid down away from a wall, or on the ground", "[WallSlideAbility]")
{
    WallSlideAbility slide(WallSlideAbilityData{});
    AbilityStates states;
    Observed standingAgainstIt = movingDown(onAWall(WallSide::Left));
    standingAgainstIt.contacts.onGround = true;

    tick(slide, InputIntentions{}, movingDown(inTheAir()), states);
    REQUIRE_FALSE(states.wallSlide.active);

    tick(slide, InputIntentions{}, standingAgainstIt, states);
    REQUIRE_FALSE(states.wallSlide.active);
    REQUIRE(states.wallSlide.velocity.y == 0.0f);
}

TEST_CASE("Going up or holding still against a wall is not a slide", "[WallSlideAbility]")
{
    WallSlideAbility slide(WallSlideAbilityData{});
    AbilityStates states;

    tick(slide, InputIntentions{}, movingDown(onAWall(WallSide::Left), -100.0f), states);
    REQUIRE_FALSE(states.wallSlide.active);

    tick(slide, InputIntentions{}, movingDown(onAWall(WallSide::Left), 0.0f), states);
    REQUIRE_FALSE(states.wallSlide.active);
}

TEST_CASE("A slide stops as soon as it is not falling against the wall", "[WallSlideAbility]")
{
    WallSlideAbility slide(WallSlideAbilityData{});
    AbilityStates states;
    tick(slide, InputIntentions{}, movingDown(onAWall(WallSide::Left)), states);

    tick(slide, InputIntentions{}, movingDown(justOffAWall(WallSide::Left)), states);

    REQUIRE_FALSE(states.wallSlide.active);
    REQUIRE(states.wallSlide.velocity.y == 0.0f);
}

TEST_CASE("A slide that does not slide is refused", "[WallSlideAbility]")
{
    WallSlideAbilityData noSpeed;
    noSpeed.slideSpeed = 0.0f;

    REQUIRE_THROWS_WITH(
        WallSlideAbility(noSpeed),
        Catch::Matchers::ContainsSubstring("A slide needs a speed above 0"));
}
