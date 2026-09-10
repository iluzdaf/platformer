#include <optional>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include "conditions/asked.hpp"
#include "conditions/facts.hpp"

namespace
{
    Facts declared()
    {
        Facts facts;
        facts["near"] = false;
        facts["hits"] = 0.0f;
        facts["mood"] = std::string("calm");
        return facts;
    }
}

TEST_CASE("A declared fact takes a value of its own kind", "[Facts]")
{
    REQUIRE_FALSE(whyNotDeclared(declared(), "near", true).has_value());
    REQUIRE_FALSE(whyNotDeclared(declared(), "hits", 3.0f).has_value());
    REQUIRE_FALSE(whyNotDeclared(declared(), "mood", std::string("angry")).has_value());
}

TEST_CASE("A fact nobody declared, or a value of the wrong kind, is said why", "[Facts]")
{
    REQUIRE(whyNotDeclared(declared(), "herd", true) == "\"herd\" is not a declared fact");
    REQUIRE(
        whyNotDeclared(declared(), "near", 3.0f) ==
        "\"near\" is a yes or no, and was given a number");
    REQUIRE(
        whyNotDeclared(declared(), "mood", false) ==
        "\"mood\" is a name, and was given a yes or no");
}

TEST_CASE("Each kind has an empty value, and every value has words", "[Facts]")
{
    REQUIRE(emptyOf(AskedKind::YesOrNo) == Asked{false});
    REQUIRE(emptyOf(AskedKind::Number) == Asked{0.0f});
    REQUIRE(emptyOf(AskedKind::Name) == Asked{std::string()});

    REQUIRE(textOf(true) == "yes");
    REQUIRE(textOf(false) == "no");
    REQUIRE(textOf(2.5f) == "2.5");
    REQUIRE(textOf(std::string("angry")) == "\"angry\"");
}
