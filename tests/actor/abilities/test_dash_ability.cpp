#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/dash_ability.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

TEST_CASE("DashAbility basic movement behavior", "[DashAbility]")
{
    Decided decided;
    Observed observed;
    InputIntentions inputIntentions;
    DashAbilityData dashAbilityData;
    DashAbility dashAbility(dashAbilityData);

    SECTION("Can dash left")
    {
        inputIntentions.direction.x = -1;
        inputIntentions.dashRequested = true;
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.velocity.x == Approx(-dashAbilityData.dashSpeed));
        REQUIRE(decided.dash.active);
        REQUIRE(decided.dash.direction == -1);
        inputIntentions = InputIntentions();
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.velocity.x == Approx(-dashAbilityData.dashSpeed));
        REQUIRE(decided.dash.active);
        REQUIRE(decided.dash.direction == -1);
    }

    SECTION("Can dash right")
    {
        inputIntentions.direction.x = 1;
        inputIntentions.dashRequested = true;
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.velocity.x == Approx(dashAbilityData.dashSpeed));
        REQUIRE(decided.dash.active);
        REQUIRE(decided.dash.direction == 1);
        inputIntentions = InputIntentions();
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.velocity.x == Approx(dashAbilityData.dashSpeed));
        REQUIRE(decided.dash.active);
        REQUIRE(decided.dash.direction == 1);
    }

    SECTION("Cannot dash if no direction given")
    {
        inputIntentions.dashRequested = true;
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.dash.active);
        REQUIRE(decided.dash.velocity.x == Approx(0.0f));
    }

    SECTION("Dash ends after duration")
    {
        inputIntentions.direction.x = -1;
        inputIntentions.dashRequested = true;
        dashAbility.decide(
            dashAbilityData.dashDuration + 0.01f, inputIntentions, observed, decided);
        REQUIRE_FALSE(decided.dash.active);
        REQUIRE(decided.dash.velocity.x == Approx(0.0f));
    }

    SECTION("Requesting to dash mid-dash should not change dashTimeLeft")
    {
        inputIntentions.direction.x = 1;
        inputIntentions.dashRequested = true;
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        inputIntentions = InputIntentions();
        inputIntentions.dashRequested = true;
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.velocity.x == Approx(dashAbilityData.dashSpeed));
        REQUIRE(decided.dash.active);
        REQUIRE(decided.dash.timeLeft == Approx(dashAbilityData.dashDuration - 0.02f));
    }

    SECTION("Dash cancels when touching wall")
    {
        inputIntentions.direction.x = -1;
        inputIntentions.dashRequested = true;
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        observed.contacts.touchingLeftWall = true;
        inputIntentions = InputIntentions();
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.velocity.x == Approx(0.0f));
        REQUIRE_FALSE(decided.dash.active);
    }

    SECTION("Cannot dash while touching wall")
    {
        observed.contacts.touchingLeftWall = true;
        inputIntentions.direction.x = 1;
        inputIntentions.dashRequested = true;
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.velocity.x == Approx(0.0f));
        REQUIRE_FALSE(decided.dash.active);
    }
}
namespace
{
    float dashRunsFor(const DashAbilityData &data, bool onGround)
    {
        Decided decided;
        Observed observed;
        DashAbility dashAbility(data);

        observed.contacts.onGround = true;
        dashAbility.decide(0.01f, InputIntentions(), observed, decided);
        observed.contacts.onGround = onGround;

        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1.0f;
        inputIntentions.dashRequested = true;
        dashAbility.decide(0.01f, inputIntentions, observed, decided);
        REQUIRE(decided.dash.active);

        float lasted = 0.01f;
        inputIntentions = InputIntentions();
        while (decided.dash.active && lasted < 2.0f)
        {
            dashAbility.decide(0.01f, inputIntentions, observed, decided);
            lasted += 0.01f;
        }
        return lasted;
    }
}

TEST_CASE("A dash begun in the air is the shorter one", "[DashAbility]")
{
    DashAbilityData data;
    data.dashDuration = 0.2f;
    data.airborneFraction = 0.5f;

    REQUIRE(dashRunsFor(data, true) == Approx(0.21f).margin(0.011f));
    REQUIRE(dashRunsFor(data, false) == Approx(0.11f).margin(0.011f));
}

TEST_CASE("A dash is the same either way until it is told otherwise", "[DashAbility]")
{
    DashAbilityData data;
    data.dashDuration = 0.2f;

    REQUIRE(data.airborneFraction == 1.0f);
    REQUIRE(dashRunsFor(data, true) == Approx(dashRunsFor(data, false)).margin(0.011f));
}

TEST_CASE("A dash refuses a fraction it cannot use", "[DashAbility]")
{
    DashAbilityData data;

    data.airborneFraction = 0.0f;
    REQUIRE_THROWS(DashAbility{data});

    data.airborneFraction = 1.5f;
    REQUIRE_THROWS(DashAbility{data});
}

TEST_CASE("A dash that goes nowhere or takes no time is refused", "[DashAbility]")
{
    DashAbilityData noSpeed;
    noSpeed.dashSpeed = 0.0f;
    REQUIRE_THROWS(DashAbility{noSpeed});

    DashAbilityData noTime;
    noTime.dashDuration = 0.0f;
    REQUIRE_THROWS(DashAbility{noTime});
}
