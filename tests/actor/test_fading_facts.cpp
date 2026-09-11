#include <catch2/catch_test_macros.hpp>
#include <string>
#include "actor/fading_facts.hpp"
#include "conditions/asked.hpp"

TEST_CASE("Nothing said lately is nothing to show", "[FadingFacts]")
{
    FadingFacts lately;

    lately.update(1.0f);

    REQUIRE(lately.all().empty());
}

TEST_CASE("What was said is held with the value it was said with", "[FadingFacts]")
{
    FadingFacts lately;

    lately.said("heard", true, 0.5f);

    REQUIRE(lately.all().at("heard").value == Asked{true});
    REQUIRE(lately.all().at("heard").secondsLeft == 0.5f);
}

TEST_CASE("What was said goes once its seconds run out", "[FadingFacts]")
{
    FadingFacts lately;
    lately.said("heard", true, 0.5f);

    lately.update(0.4f);
    REQUIRE(lately.all().contains("heard"));

    lately.update(0.1f);
    REQUIRE(lately.all().empty());
}

TEST_CASE("Saying it again gives it its seconds back", "[FadingFacts]")
{
    FadingFacts lately;
    lately.said("heard", true, 0.5f);
    lately.update(0.4f);

    lately.said("heard", false, 0.5f);
    lately.update(0.2f);

    REQUIRE(lately.all().at("heard").value == Asked{false});
    REQUIRE(lately.all().at("heard").secondsLeft > 0.0f);
}
