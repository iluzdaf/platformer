#include <catch2/catch_test_macros.hpp>
#include <map>
#include <string>
#include "serialization/only_what_differs.hpp"
#include "ui/saved_in_scope.hpp"

namespace saved
{
    struct CarData
    {
        std::string name;
    };
}

TEST_CASE("A field of something the saved copy never had has changed", "[SavedInScope]")
{
    std::map<std::string, saved::CarData> cars{{"rusty", saved::CarData{}}};
    SavedInScope was(asItWasSaved<std::map<std::string, saved::CarData>>(onlyWhatDiffers(cars)));

    cars["shiny"] = saved::CarData{};

    inspector::InField added("shiny");
    REQUIRE(inspector::changedHere(cars.at("shiny")));

    inspector::InField name("name");
    REQUIRE(inspector::changedHere(cars.at("shiny").name));
}

TEST_CASE("What was saved is out of scope once the scope is", "[SavedInScope]")
{
    saved::CarData car;
    {
        SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));
        car.name = "shiny";
        REQUIRE(inspector::changedHere(car));
    }

    REQUIRE_FALSE(inspector::changedHere(car));
}
