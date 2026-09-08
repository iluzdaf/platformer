#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/melee_ability.hpp"
#include "actor/abilities/melee_ability_data.hpp"
#include "actor/abilities/melee_ability_state.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

using Catch::Approx;

namespace
{
    constexpr float Step = 0.01f;

    MeleeAbilityData aSwingOf(float windup, float active, float recovery)
    {
        MeleeAbilityData data;
        data.windup = windup;
        data.active = active;
        data.recovery = recovery;
        return data;
    }

    InputIntentions pressingAttack(float x = 0.0f)
    {
        InputIntentions intentions;
        intentions.attackRequested = true;
        intentions.direction.x = x;
        return intentions;
    }
}

TEST_CASE("A swing winds up, strikes, recovers and rests", "[MeleeAbility]")
{
    Decided decided;
    Observed observed;
    InputIntentions nothing;
    MeleeAbility ability(aSwingOf(0.05f, 0.1f, 0.05f));

    ability.decide(Step, pressingAttack(), observed, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Windup);
    REQUIRE(decided.melee.swinging());
    REQUIRE_FALSE(decided.melee.striking());

    ability.decide(0.05f, nothing, observed, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Active);
    REQUIRE(decided.melee.striking());

    ability.decide(0.1f, nothing, observed, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Recovery);
    REQUIRE_FALSE(decided.melee.striking());

    ability.decide(0.05f, nothing, observed, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Idle);
    REQUIRE_FALSE(decided.melee.swinging());
}

TEST_CASE("A phase shorter than a step still gets a step, so the strike is seen", "[MeleeAbility]")
{
    Decided decided;
    Observed observed;
    InputIntentions nothing;
    MeleeAbility ability(aSwingOf(0.0f, 0.001f, 0.0f));

    ability.decide(Step, pressingAttack(), observed, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Windup);

    ability.decide(Step, nothing, observed, decided);
    REQUIRE(decided.melee.striking());

    ability.decide(Step, nothing, observed, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Recovery);

    ability.decide(Step, nothing, observed, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Idle);
}

TEST_CASE("A long phase keeps exact time, and the remainder feeds the next", "[MeleeAbility]")
{
    Decided decided;
    Observed observed;
    InputIntentions nothing;
    MeleeAbility ability(aSwingOf(0.05f, 0.1f, 0.05f));

    ability.decide(Step, pressingAttack(), observed, decided);
    ability.decide(0.06f, nothing, observed, decided);

    REQUIRE(decided.melee.phase == MeleePhase::Active);
    REQUIRE(decided.melee.timeLeft == Approx(0.09f));
}

TEST_CASE("A swing says so once, and pressing again mid-swing starts nothing", "[MeleeAbility]")
{
    Decided decided;
    Observed observed;
    MeleeAbility ability(aSwingOf(0.05f, 0.1f, 0.05f));

    ability.decide(Step, pressingAttack(), observed, decided);
    REQUIRE(decided.melee.emit);

    ability.decide(Step, pressingAttack(), observed, decided);
    REQUIRE_FALSE(decided.melee.emit);
    REQUIRE(decided.melee.phase == MeleePhase::Windup);
    REQUIRE(decided.melee.timeLeft == Approx(0.04f));
}

TEST_CASE("A swing goes where it was asked, else where the actor faces", "[MeleeAbility]")
{
    MeleeAbility ability{MeleeAbilityData{}};
    Observed facingLeft;
    facingLeft.facingLeft = true;

    SECTION("Asked left")
    {
        Decided decided;
        ability.decide(Step, pressingAttack(-1.0f), Observed{}, decided);
        REQUIRE(decided.melee.direction == -1.0f);
    }

    SECTION("Asked right while facing left")
    {
        Decided decided;
        ability.decide(Step, pressingAttack(1.0f), facingLeft, decided);
        REQUIRE(decided.melee.direction == 1.0f);
    }

    SECTION("Not asked, facing left")
    {
        Decided decided;
        ability.decide(Step, pressingAttack(), facingLeft, decided);
        REQUIRE(decided.melee.direction == -1.0f);
    }

    SECTION("Not asked, facing right")
    {
        Decided decided;
        ability.decide(Step, pressingAttack(), Observed{}, decided);
        REQUIRE(decided.melee.direction == 1.0f);
    }
}

TEST_CASE(
    "A swing carries its reach and damage, and forgets what the last one struck",
    "[MeleeAbility]")
{
    Decided decided;
    Observed observed;
    MeleeAbilityData data;
    data.reach = glm::vec2(20.0f, 6.0f);
    data.damage = 2;
    MeleeAbility ability(data);
    decided.melee.struck.push_back(nullptr);

    ability.decide(Step, pressingAttack(), observed, decided);

    REQUIRE(decided.melee.reach == glm::vec2(20.0f, 6.0f));
    REQUIRE(decided.melee.damage == 2);
    REQUIRE(decided.melee.struck.empty());
}

TEST_CASE("A swing cannot start while dashing, and a knockback cuts one short", "[MeleeAbility]")
{
    Decided decided;
    Observed observed;
    InputIntentions nothing;
    MeleeAbility ability(aSwingOf(0.05f, 0.1f, 0.05f));

    decided.dash.active = true;
    ability.decide(Step, pressingAttack(), observed, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Idle);
    REQUIRE_FALSE(decided.melee.emit);

    decided.dash.active = false;
    ability.decide(Step, pressingAttack(), observed, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Windup);

    decided.knockback.active = true;
    ability.decide(Step, nothing, observed, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Idle);
    REQUIRE(decided.melee.timeLeft == 0.0f);
}

TEST_CASE("A swing with nothing to it is refused", "[MeleeAbility]")
{
    REQUIRE_THROWS(MeleeAbility(aSwingOf(-0.1f, 0.1f, 0.1f)));
    REQUIRE_THROWS(MeleeAbility(aSwingOf(0.1f, 0.0f, 0.1f)));
    REQUIRE_THROWS(MeleeAbility(aSwingOf(0.1f, 0.1f, -0.1f)));
    REQUIRE_NOTHROW(MeleeAbility(aSwingOf(0.0f, 0.1f, 0.0f)));

    MeleeAbilityData noReach;
    noReach.reach = glm::vec2(0.0f, 10.0f);
    REQUIRE_THROWS(MeleeAbility(noReach));

    MeleeAbilityData harmless;
    harmless.damage = 0;
    REQUIRE_THROWS(MeleeAbility(harmless));
}
