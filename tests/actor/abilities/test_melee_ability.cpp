#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <string>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/melee_ability.hpp"
#include "actor/abilities/melee_ability_data.hpp"
#include "actor/abilities/melee_ability_state.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"

namespace
{
    constexpr float Step = 0.01f;

    InputIntentions pressingAttack(float x = 0.0f)
    {
        InputIntentions intentions;
        intentions.attackRequested = true;
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
    "[MeleeAbility]")
{
    Decided decided;
    InputIntentions nothing;
    MeleeAbility ability(MeleeAbilityData{});

    ability.decide(Step, pressingAttack(), Observed{}, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Windup);
    REQUIRE(decided.melee.swinging());
    REQUIRE_FALSE(decided.melee.striking());

    ability.decide(Step, nothing, clipSaying(std::string(StrikeCue)), decided);
    REQUIRE(decided.melee.striking());

    ability.decide(Step, nothing, clipSaying(std::string(RecoverCue)), decided);
    REQUIRE(decided.melee.phase == MeleePhase::Recovery);
    REQUIRE(decided.melee.swinging());

    ability.decide(Step, nothing, clipFinished(), decided);
    REQUIRE(decided.melee.phase == MeleePhase::Idle);
    REQUIRE_FALSE(decided.melee.swinging());
}

TEST_CASE(
    "A swing strikes on the strike cue and not before, however long the wait",
    "[MeleeAbility]")
{
    Decided decided;
    InputIntentions nothing;
    MeleeAbility ability(MeleeAbilityData{});
    ability.decide(Step, pressingAttack(), Observed{}, decided);

    for (int step = 0; step < 100; ++step)
        ability.decide(Step, nothing, Observed{}, decided);
    REQUIRE(decided.melee.phase == MeleePhase::Windup);

    ability.decide(Step, nothing, clipSaying(std::string(StrikeCue)), decided);
    REQUIRE(decided.melee.striking());
}

TEST_CASE("A swing ends with its clip even mid-strike", "[MeleeAbility]")
{
    Decided decided;
    InputIntentions nothing;
    MeleeAbility ability(MeleeAbilityData{});
    ability.decide(Step, pressingAttack(), Observed{}, decided);
    ability.decide(Step, nothing, clipSaying(std::string(StrikeCue)), decided);

    ability.decide(Step, nothing, clipFinished(), decided);

    REQUIRE(decided.melee.phase == MeleePhase::Idle);
}

TEST_CASE("A swing says so once, and pressing again mid-swing starts nothing", "[MeleeAbility]")
{
    Decided decided;
    MeleeAbility ability(MeleeAbilityData{});

    ability.decide(Step, pressingAttack(), Observed{}, decided);
    REQUIRE(decided.melee.emit);

    ability.decide(Step, pressingAttack(), clipSaying(std::string(StrikeCue)), decided);
    REQUIRE_FALSE(decided.melee.emit);
    REQUIRE(decided.melee.striking());

    ability.decide(Step, pressingAttack(), Observed{}, decided);
    REQUIRE_FALSE(decided.melee.emit);
    REQUIRE(decided.melee.striking());
}

TEST_CASE("A swing goes where it was asked, else where the actor faces", "[MeleeAbility]")
{
    MeleeAbility ability(MeleeAbilityData{});

    Decided asked;
    ability.decide(Step, pressingAttack(-1.0f), Observed{}, asked);
    REQUIRE(asked.melee.direction == -1.0f);

    Decided facing;
    ability.decide(Step, pressingAttack(), facingLeft(), facing);
    REQUIRE(facing.melee.direction == -1.0f);

    Decided ahead;
    ability.decide(Step, pressingAttack(), Observed{}, ahead);
    REQUIRE(ahead.melee.direction == 1.0f);
}

TEST_CASE("A swing carries its reach and damage and forgets who it struck", "[MeleeAbility]")
{
    MeleeAbilityData data;
    data.reach = glm::vec2(20.0f, 6.0f);
    data.damage = 3;
    MeleeAbility ability(data);
    Decided decided;
    decided.melee.struck.push_back(nullptr);

    ability.decide(Step, pressingAttack(), Observed{}, decided);

    REQUIRE(decided.melee.reach == glm::vec2(20.0f, 6.0f));
    REQUIRE(decided.melee.damage == 3);
    REQUIRE(decided.melee.struck.empty());
}

TEST_CASE("A swing cannot start while dashing, and a knockback cuts one short", "[MeleeAbility]")
{
    MeleeAbility ability(MeleeAbilityData{});

    Decided dashing;
    dashing.dash.active = true;
    ability.decide(Step, pressingAttack(), Observed{}, dashing);
    REQUIRE(dashing.melee.phase == MeleePhase::Idle);

    Decided knocked;
    ability.decide(Step, pressingAttack(), Observed{}, knocked);
    ability.decide(Step, InputIntentions{}, clipSaying(std::string(StrikeCue)), knocked);
    REQUIRE(knocked.melee.striking());
    knocked.knockback.active = true;
    ability.decide(Step, InputIntentions{}, Observed{}, knocked);
    REQUIRE(knocked.melee.phase == MeleePhase::Idle);
}

TEST_CASE("A swing with nothing to it is refused", "[MeleeAbility]")
{
    MeleeAbilityData noReach;
    noReach.reach = glm::vec2(0.0f, 10.0f);
    REQUIRE_THROWS_WITH(MeleeAbility(noReach), Catch::Matchers::ContainsSubstring("reach"));

    MeleeAbilityData noDamage;
    noDamage.damage = 0;
    REQUIRE_THROWS_WITH(MeleeAbility(noDamage), Catch::Matchers::ContainsSubstring("damage"));
}
