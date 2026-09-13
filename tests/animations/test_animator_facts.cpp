#include <string>
#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/ability_states.hpp"
#include "actor/actor_facts.hpp"
#include "actor/actor_fact_rows.hpp"
#include "actor/observed.hpp"
#include "animations/animator_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "helpers/actor_facts.hpp"

TEST_CASE(
    "An animation may ask anything about the actor, then about its picture",
    "[AnimatorFacts]")
{
    std::string actors;
    for (const FactRow<ActorFacts> &row : actorRows())
        actors += std::string(row.name) + " ";

    std::string names;
    for (const FactRow<ActorFacts> &row : animatorRows())
        names += std::string(row.name) + " ";

    REQUIRE(names == actors + "finished inState ");
    REQUIRE(rowNamed(animatorRows(), "inState")->kind == AskedKind::Name);
}

TEST_CASE("Finished follows the clip, and in state follows the actor", "[AnimatorFacts]")
{
    AbilityStates states;
    Observed observed;
    ActorFacts facts = factsOf(states, observed, "sleep");
    const FactRow<ActorFacts> &finished = *rowNamed(animatorRows(), "finished");
    const FactRow<ActorFacts> &inState = *rowNamed(animatorRows(), "inState");

    REQUIRE(finished.holds(false, facts));
    facts.finished = true;
    REQUIRE(finished.holds(true, facts));

    REQUIRE(inState.holds(std::string("sleep"), facts));
    REQUIRE_FALSE(inState.holds(std::string("charge"), facts));
}
