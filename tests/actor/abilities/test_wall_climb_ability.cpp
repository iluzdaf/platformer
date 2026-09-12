#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/wall_climb_ability.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/decided.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    Decided hanging()
    {
        Decided decided;
        decided.wallHang.active = true;
        return decided;
    }
}

TEST_CASE("Hanging on a wall, a climb goes up and down at its speed", "[WallClimbAbility]")
{
    WallClimbAbilityData data;
    WallClimbAbility climb(data);
    Decided decided = hanging();

    tick(climb, pressing(0.0f, -1.0f), onAWall(WallSide::Left), decided);
    REQUIRE(decided.wallClimb.velocity.y == Approx(-data.climbSpeed));

    tick(climb, pressing(0.0f, 1.0f), onAWall(WallSide::Left), decided);
    REQUIRE(decided.wallClimb.velocity.y == Approx(data.climbSpeed));
}

TEST_CASE("However little it is pressed, a climb goes at its full speed", "[WallClimbAbility]")
{
    WallClimbAbilityData data;
    WallClimbAbility climb(data);
    Decided decided = hanging();

    tick(climb, pressing(0.0f, -0.3f), onAWall(WallSide::Left), decided);
    REQUIRE(decided.wallClimb.velocity.y == Approx(-data.climbSpeed));
}

TEST_CASE("Hanging with nothing pressed, a climb holds still", "[WallClimbAbility]")
{
    WallClimbAbility climb(WallClimbAbilityData{});
    Decided decided = hanging();
    tick(climb, pressing(0.0f, -1.0f), onAWall(WallSide::Left), decided);

    tick(climb, InputIntentions{}, onAWall(WallSide::Left), decided);

    REQUIRE(decided.wallClimb.velocity == glm::vec2(0.0f));
}

TEST_CASE("Without hanging on, nothing is climbed either way", "[WallClimbAbility]")
{
    WallClimbAbility climb(WallClimbAbilityData{});
    Decided decided;

    tick(climb, pressing(0.0f, -1.0f), onAWall(WallSide::Left), decided);
    REQUIRE(decided.wallClimb.velocity.y == 0.0f);

    tick(climb, pressing(0.0f, 1.0f), onAWall(WallSide::Left), decided);
    REQUIRE(decided.wallClimb.velocity.y == 0.0f);
}

TEST_CASE("A climb that goes nowhere is refused", "[WallClimbAbility]")
{
    WallClimbAbilityData noSpeed;
    noSpeed.climbSpeed = 0.0f;

    REQUIRE_THROWS_WITH(
        WallClimbAbility(noSpeed), Catch::Matchers::ContainsSubstring("climbSpeed"));
}
