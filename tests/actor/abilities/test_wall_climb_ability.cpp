#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/wall_climb_ability.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("Hanging on a wall, a climb goes up and down at its speed", "[WallClimbAbility]")
{
    WallClimbAbilityData data;
    WallClimbAbility climb(data);
    AbilityStates states = hanging();

    tick(climb, pressingUp(), onAWall(WallSide::Left), states);
    REQUIRE(states.wallClimb.velocity.y == Approx(-data.climbSpeed));

    tick(climb, pressingDown(), onAWall(WallSide::Left), states);
    REQUIRE(states.wallClimb.velocity.y == Approx(data.climbSpeed));
}

TEST_CASE("However little it is pressed, a climb goes at its full speed", "[WallClimbAbility]")
{
    WallClimbAbilityData data;
    WallClimbAbility climb(data);
    AbilityStates states = hanging();

    tick(climb, pressing(0.0f, -0.3f), onAWall(WallSide::Left), states);
    REQUIRE(states.wallClimb.velocity.y == Approx(-data.climbSpeed));
}

TEST_CASE("Hanging with nothing pressed, a climb holds still", "[WallClimbAbility]")
{
    WallClimbAbility climb(WallClimbAbilityData{});
    AbilityStates states = hanging();
    tick(climb, pressingUp(), onAWall(WallSide::Left), states);

    tick(climb, InputIntentions{}, onAWall(WallSide::Left), states);

    REQUIRE(states.wallClimb.velocity == glm::vec2(0.0f));
}

TEST_CASE("Without hanging on, nothing is climbed either way", "[WallClimbAbility]")
{
    WallClimbAbility climb(WallClimbAbilityData{});
    AbilityStates states;

    tick(climb, pressingUp(), onAWall(WallSide::Left), states);
    REQUIRE(states.wallClimb.velocity.y == 0.0f);

    tick(climb, pressingDown(), onAWall(WallSide::Left), states);
    REQUIRE(states.wallClimb.velocity.y == 0.0f);
}

TEST_CASE("A climb that goes nowhere is refused", "[WallClimbAbility]")
{
    WallClimbAbilityData noSpeed;
    noSpeed.climbSpeed = 0.0f;

    REQUIRE_THROWS_WITH(
        WallClimbAbility(noSpeed),
        Catch::Matchers::ContainsSubstring("A climb needs a speed above 0"));
}
