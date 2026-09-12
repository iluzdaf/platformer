#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/abilities/dash_ability.hpp"
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "helpers/abilities.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    DashAbilityData timedAs(float duration, float airborneFraction = 1.0f)
    {
        DashAbilityData data;
        data.dashDuration = duration;
        data.airborneFraction = airborneFraction;
        return data;
    }

    bool ended(const Decided &decided)
    {
        return !decided.dash.active;
    }

    int ticksADashLasts(const DashAbilityData &data, const Observed &from)
    {
        DashAbility dash(data);
        Decided decided;
        tick(dash, pressingDash(1.0f), from, decided);
        return ticksUntil(dash, InputIntentions{}, from, decided, ended);
    }
}

TEST_CASE("A dash sets off the way it is pressed, at its speed, and keeps going", "[DashAbility]")
{
    DashAbilityData data;

    DashAbility right(data);
    Decided goingRight;
    tick(right, pressingDash(1.0f), onTheGround(), goingRight);
    REQUIRE(goingRight.dash.active);
    REQUIRE(goingRight.dash.velocity.x == Approx(data.dashSpeed));

    tick(right, InputIntentions{}, onTheGround(), goingRight);
    REQUIRE(goingRight.dash.velocity.x == Approx(data.dashSpeed));

    DashAbility left(data);
    Decided goingLeft;
    tick(left, pressingDash(-1.0f), onTheGround(), goingLeft);
    REQUIRE(goingLeft.dash.velocity.x == Approx(-data.dashSpeed));

    tick(left, InputIntentions{}, onTheGround(), goingLeft);
    REQUIRE(goingLeft.dash.velocity.x == Approx(-data.dashSpeed));
}

TEST_CASE("A dash says so once", "[DashAbility]")
{
    DashAbility dash(DashAbilityData{});
    Decided decided;

    tick(dash, pressingDash(1.0f), onTheGround(), decided);
    REQUIRE(decided.dash.emit);

    tick(dash, InputIntentions{}, onTheGround(), decided);
    REQUIRE(decided.dash.active);
    REQUIRE_FALSE(decided.dash.emit);
}

TEST_CASE("A dash needs a direction to go in", "[DashAbility]")
{
    DashAbility dash(DashAbilityData{});
    Decided decided;

    tick(dash, pressingDash(0.0f), onTheGround(), decided);

    REQUIRE_FALSE(decided.dash.active);
    REQUIRE(decided.dash.velocity.x == 0.0f);
}

TEST_CASE("A dash lasts as long as it says, and then ends", "[DashAbility]")
{
    DashAbility dash(timedAs(0.045f));
    Decided decided;
    tick(dash, pressingDash(1.0f), onTheGround(), decided);

    int ticksUntilItEnds = ticksUntil(dash, InputIntentions{}, onTheGround(), decided, ended);

    REQUIRE(ticksUntilItEnds == 4);
    REQUIRE(decided.dash.velocity.x == 0.0f);
}

TEST_CASE("A dash begun in the air is the shorter one", "[DashAbility]")
{
    DashAbilityData data = timedAs(0.045f, 0.5f);

    REQUIRE(ticksADashLasts(data, onTheGround()) == 4);
    REQUIRE(ticksADashLasts(data, inTheAir()) == 2);
}

TEST_CASE("A dash is the same either way until it is told otherwise", "[DashAbility]")
{
    DashAbilityData data = timedAs(0.045f);

    REQUIRE(DashAbilityData{}.airborneFraction == 1.0f);
    REQUIRE(ticksADashLasts(data, inTheAir()) == ticksADashLasts(data, onTheGround()));
}

TEST_CASE("Pressing dash again mid-dash does not make it last longer", "[DashAbility]")
{
    DashAbilityData data = timedAs(0.045f);

    DashAbility pressedOnce(data);
    Decided once;
    tick(pressedOnce, pressingDash(1.0f), onTheGround(), once);
    int onceFor = ticksUntil(pressedOnce, InputIntentions{}, onTheGround(), once, ended);

    DashAbility pressedAgain(data);
    Decided again;
    tick(pressedAgain, pressingDash(1.0f), onTheGround(), again);
    int againFor = ticksUntil(pressedAgain, pressingDash(1.0f), onTheGround(), again, ended);

    REQUIRE(againFor == onceFor);
}

TEST_CASE("A dash ends when it meets a wall", "[DashAbility]")
{
    DashAbility dash(DashAbilityData{});
    Decided decided;
    tick(dash, pressingDash(-1.0f), inTheAir(), decided);

    tick(dash, InputIntentions{}, onAWall(WallSide::Left), decided);

    REQUIRE_FALSE(decided.dash.active);
    REQUIRE(decided.dash.velocity.x == 0.0f);
}

TEST_CASE(
    "A dash cannot start against a wall, even pressing away from it, and is not used up trying",
    "[DashAbility]")
{
    DashAbility dash(DashAbilityData{});
    Decided decided;

    tick(dash, pressingDash(-1.0f), onAWall(WallSide::Left), decided);
    REQUIRE_FALSE(decided.dash.active);
    REQUIRE_FALSE(decided.dash.emit);

    tick(dash, pressingDash(1.0f), onAWall(WallSide::Left), decided);
    REQUIRE_FALSE(decided.dash.active);
    REQUIRE_FALSE(decided.dash.emit);
    REQUIRE(decided.dash.velocity.x == 0.0f);

    tick(dash, pressingDash(1.0f), inTheAir(), decided);
    REQUIRE(decided.dash.active);
}

TEST_CASE("A dash taken in the air is not taken again until the ground is touched", "[DashAbility]")
{
    DashAbility dash(timedAs(0.045f));
    Decided decided;
    tick(dash, pressingDash(1.0f), inTheAir(), decided);
    ticksUntil(dash, InputIntentions{}, inTheAir(), decided, ended);

    tick(dash, pressingDash(1.0f), inTheAir(), decided);
    REQUIRE_FALSE(decided.dash.active);

    tick(dash, InputIntentions{}, onTheGround(), decided);
    tick(dash, pressingDash(1.0f), inTheAir(), decided);
    REQUIRE(decided.dash.active);
}

TEST_CASE("On the ground a dash can be taken again as soon as one ends", "[DashAbility]")
{
    DashAbility dash(timedAs(0.045f));
    Decided decided;
    tick(dash, pressingDash(1.0f), onTheGround(), decided);
    ticksUntil(dash, InputIntentions{}, onTheGround(), decided, ended);

    tick(dash, pressingDash(1.0f), onTheGround(), decided);

    REQUIRE(decided.dash.active);
}

TEST_CASE("A dash that goes nowhere or takes no time is refused", "[DashAbility]")
{
    DashAbilityData noSpeed;
    noSpeed.dashSpeed = 0.0f;
    REQUIRE_THROWS_WITH(DashAbility{noSpeed}, Catch::Matchers::ContainsSubstring("dashSpeed"));

    REQUIRE_THROWS_WITH(
        DashAbility{timedAs(0.0f)}, Catch::Matchers::ContainsSubstring("dashDuration"));
}

TEST_CASE("A dash refuses a fraction it cannot use", "[DashAbility]")
{
    REQUIRE_THROWS_WITH(
        DashAbility{timedAs(0.2f, 0.0f)}, Catch::Matchers::ContainsSubstring("airborneFraction"));
    REQUIRE_THROWS_WITH(
        DashAbility{timedAs(0.2f, 1.5f)}, Catch::Matchers::ContainsSubstring("airborneFraction"));
}
