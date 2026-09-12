#include <string>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "actor/declared_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/facts.hpp"

using Catch::Matchers::ContainsSubstring;

namespace
{
    FactsData aListener()
    {
        FactsData declared;
        declared["heard"] = false;
        declared["mood"] = std::string("calm");
        return declared;
    }
}

TEST_CASE("Declared facts start as they were declared, with nothing said", "[DeclaredFacts]")
{
    DeclaredFacts facts(aListener());

    REQUIRE(facts.all() == aListener());
    REQUIRE(facts.fact("mood") == Asked{std::string("calm")});
    REQUIRE(facts.saidLately().all().empty());
}

TEST_CASE("A fact said stays said past the end of the tick", "[DeclaredFacts]")
{
    DeclaredFacts facts(aListener());

    facts.fact("mood", std::string("angry"));
    facts.forgetTheTick();

    REQUIRE(facts.fact("mood") == Asked{std::string("angry")});
    REQUIRE(facts.saidLately().all().empty());
}

TEST_CASE(
    "An event holds for the tick it is said in, then is what was declared again",
    "[DeclaredFacts]")
{
    DeclaredFacts facts(aListener());

    facts.event("heard", true);
    REQUIRE(facts.fact("heard") == Asked{true});

    facts.forgetTheTick();
    REQUIRE(facts.fact("heard") == Asked{false});
}

TEST_CASE("What an event said lingers for half a second, then is gone", "[DeclaredFacts]")
{
    DeclaredFacts facts(aListener());
    facts.event("heard", true);
    facts.forgetTheTick();

    facts.fade(0.45f);
    REQUIRE(facts.saidLately().all().at("heard").value == Asked{true});

    facts.fade(0.1f);
    REQUIRE(facts.saidLately().all().empty());
}

TEST_CASE("A fact nobody declared is refused, whether said or asked", "[DeclaredFacts]")
{
    DeclaredFacts facts(aListener());

    REQUIRE_THROWS_WITH(
        facts.fact("herd", true), ContainsSubstring("\"herd\" is not a declared fact"));
    REQUIRE_THROWS_WITH(
        facts.event("herd", true), ContainsSubstring("\"herd\" is not a declared fact"));
    REQUIRE_THROWS_WITH(facts.fact("herd"), ContainsSubstring("\"herd\" is not a declared fact"));
    REQUIRE(facts.all() == aListener());
}

TEST_CASE("A fact said as the wrong kind is refused", "[DeclaredFacts]")
{
    DeclaredFacts facts(aListener());

    REQUIRE_THROWS_WITH(
        facts.fact("heard", 3.0f),
        ContainsSubstring("\"heard\" is a yes or no, and was given a number"));
    REQUIRE_THROWS_WITH(
        facts.event("mood", true),
        ContainsSubstring("\"mood\" is a name, and was given a yes or no"));
    REQUIRE(facts.all() == aListener());
}

TEST_CASE("A fact the engine answers cannot be declared", "[DeclaredFacts]")
{
    FactsData declared = aListener();
    declared["onGround"] = true;

    REQUIRE_THROWS_WITH(
        DeclaredFacts(declared),
        ContainsSubstring("\"onGround\" is a fact the engine answers, and cannot be declared"));
}
