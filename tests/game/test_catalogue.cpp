#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <map>
#include <string>
#include "game/catalogue.hpp"

TEST_CASE("A catalogue hands back what it was asked for", "[Catalogue]")
{
    std::map<std::string, int> catalogue{{"coin", 10}, {"gem", 50}};

    REQUIRE(oneNamed(catalogue, "pickup", "gem") == 50);
}

TEST_CASE("A catalogue says what was asked for and what kind", "[Catalogue]")
{
    std::map<std::string, int> catalogue{{"coin", 10}};

    REQUIRE_THROWS_WITH(
        oneNamed(catalogue, "pickup", "gem"),
        Catch::Matchers::ContainsSubstring("pickup") && Catch::Matchers::ContainsSubstring("gem"));
}

TEST_CASE("An empty catalogue names nothing at all", "[Catalogue]")
{
    std::map<std::string, int> catalogue;

    REQUIRE_THROWS_WITH(
        oneNamed(catalogue, "npc", "villager"), Catch::Matchers::ContainsSubstring("villager"));
}
