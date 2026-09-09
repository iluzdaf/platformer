#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <string>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/swing_ability.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

namespace
{
    constexpr float Step = 0.01f;

    InputIntentions pressingAttack(float x = 0.0f)
    {
        InputIntentions intentions;
        intentions.attack = std::string(SwingAttack);
        intentions.direction.x = x;
        return intentions;
    }

    Observed clipSaying(std::string cue)
    {
        Observed observed;
        observed.cues.push_back(std::move(cue));
        return observed;
    }

    Observed clipFinished()
    {
        Observed observed;
        observed.animationFinished = true;
        return observed;
    }

    Observed facingLeft()
    {
        Observed observed;
        observed.facingLeft = true;
        return observed;
    }
}

TEST_CASE(
    "A swing winds up, strikes on cue, recovers on cue and rests when the clip ends",
    "[SwingAbility]")
{
    Decided decided;
    InputIntentions nothing;
    SwingAbility ability(SwingAbilityData{});

    ability.decide(Step, pressingAttack(), Observed{}, decided);
    REQUIRE(decided.swing.phase == SwingPhase::Windup);
    REQUIRE(decided.swing.swinging());
    REQUIRE_FALSE(decided.swing.striking());

    ability.decide(Step, nothing, clipSaying(std::string(StrikeCue)), decided);
    REQUIRE(decided.swing.striking());

    ability.decide(Step, nothing, clipSaying(std::string(RecoverCue)), decided);
    REQUIRE(decided.swing.phase == SwingPhase::Recovery);
    REQUIRE(decided.swing.swinging());

    ability.decide(Step, nothing, clipFinished(), decided);
    REQUIRE(decided.swing.phase == SwingPhase::Idle);
    REQUIRE_FALSE(decided.swing.swinging());
}

TEST_CASE(
    "A swing strikes on the strike cue and not before, however long the wait",
    "[SwingAbility]")
{
    Decided decided;
    InputIntentions nothing;
    SwingAbility ability(SwingAbilityData{});
    ability.decide(Step, pressingAttack(), Observed{}, decided);

    for (int step = 0; step < 100; ++step)
        ability.decide(Step, nothing, Observed{}, decided);
    REQUIRE(decided.swing.phase == SwingPhase::Windup);

    ability.decide(Step, nothing, clipSaying(std::string(StrikeCue)), decided);
    REQUIRE(decided.swing.striking());
}

TEST_CASE("A swing ends with its clip even mid-strike", "[SwingAbility]")
{
    Decided decided;
    InputIntentions nothing;
    SwingAbility ability(SwingAbilityData{});
    ability.decide(Step, pressingAttack(), Observed{}, decided);
    ability.decide(Step, nothing, clipSaying(std::string(StrikeCue)), decided);

    ability.decide(Step, nothing, clipFinished(), decided);

    REQUIRE(decided.swing.phase == SwingPhase::Idle);
}

TEST_CASE("A swing says so once, and pressing again mid-swing starts nothing", "[SwingAbility]")
{
    Decided decided;
    SwingAbility ability(SwingAbilityData{});

    ability.decide(Step, pressingAttack(), Observed{}, decided);
    REQUIRE(decided.swing.emit);

    ability.decide(Step, pressingAttack(), clipSaying(std::string(StrikeCue)), decided);
    REQUIRE_FALSE(decided.swing.emit);
    REQUIRE(decided.swing.striking());

    ability.decide(Step, pressingAttack(), Observed{}, decided);
    REQUIRE_FALSE(decided.swing.emit);
    REQUIRE(decided.swing.striking());
}

TEST_CASE("A swing goes where it was asked, else where the actor faces", "[SwingAbility]")
{
    SwingAbility ability(SwingAbilityData{});

    Decided asked;
    ability.decide(Step, pressingAttack(-1.0f), Observed{}, asked);
    REQUIRE(asked.swing.direction == -1.0f);

    Decided facing;
    ability.decide(Step, pressingAttack(), facingLeft(), facing);
    REQUIRE(facing.swing.direction == -1.0f);

    Decided ahead;
    ability.decide(Step, pressingAttack(), Observed{}, ahead);
    REQUIRE(ahead.swing.direction == 1.0f);
}

TEST_CASE("A swing carries its reach and damage and forgets who it struck", "[SwingAbility]")
{
    SwingAbilityData data;
    data.reach = glm::vec2(20.0f, 6.0f);
    data.damage = 3;
    SwingAbility ability(data);
    Decided decided;
    decided.swing.struck.push_back(nullptr);

    ability.decide(Step, pressingAttack(), Observed{}, decided);

    REQUIRE(decided.swing.reach == glm::vec2(20.0f, 6.0f));
    REQUIRE(decided.swing.damage == 3);
    REQUIRE(decided.swing.struck.empty());
}

TEST_CASE("A swing cannot start while dashing, and a knockback cuts one short", "[SwingAbility]")
{
    SwingAbility ability(SwingAbilityData{});

    Decided dashing;
    dashing.dash.active = true;
    ability.decide(Step, pressingAttack(), Observed{}, dashing);
    REQUIRE(dashing.swing.phase == SwingPhase::Idle);

    Decided knocked;
    ability.decide(Step, pressingAttack(), Observed{}, knocked);
    ability.decide(Step, InputIntentions{}, clipSaying(std::string(StrikeCue)), knocked);
    REQUIRE(knocked.swing.striking());
    knocked.knockback.active = true;
    ability.decide(Step, InputIntentions{}, Observed{}, knocked);
    REQUIRE(knocked.swing.phase == SwingPhase::Idle);
}

TEST_CASE("A swing with nothing to it is refused", "[SwingAbility]")
{
    SwingAbilityData noReach;
    noReach.reach = glm::vec2(0.0f, 10.0f);
    REQUIRE_THROWS_WITH(SwingAbility(noReach), Catch::Matchers::ContainsSubstring("reach"));

    SwingAbilityData noDamage;
    noDamage.damage = 0;
    REQUIRE_THROWS_WITH(SwingAbility(noDamage), Catch::Matchers::ContainsSubstring("damage"));
}

TEST_CASE("A swing is not started by an attack that is not a swing", "[SwingAbility]")
{
    SwingAbility ability(SwingAbilityData{});
    Decided decided;
    InputIntentions pouncing;
    pouncing.attack = "pounce";

    ability.decide(Step, pouncing, Observed{}, decided);

    REQUIRE(decided.swing.phase == SwingPhase::Idle);
}
