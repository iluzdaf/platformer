#include <catch2/catch_test_macros.hpp>
#include <map>
#include <string>
#include "serialization/only_what_differs.hpp"
#include "ui/saved_in_scope.hpp"

namespace saved
{
    struct WheelsData
    {
        int count = 4;
        float grip = 1.0f;
    };

    struct CarData
    {
        std::string name;
        WheelsData wheels;
    };
}

TEST_CASE("With nothing saved in scope, nothing has changed", "[SavedInScope]")
{
    saved::CarData car;
    car.name = "rusty";

    REQUIRE_FALSE(inspector::changedHere(car));
}

TEST_CASE("A field that matches what was saved has not changed", "[SavedInScope]")
{
    saved::CarData car;
    car.name = "rusty";
    SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));

    REQUIRE_FALSE(inspector::changedHere(car));

    inspector::InField name("name");
    REQUIRE_FALSE(inspector::changedHere(car.name));
}

TEST_CASE("A field edited away from what was saved has changed", "[SavedInScope]")
{
    saved::CarData car;
    car.name = "rusty";
    SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));

    car.name = "shiny";

    REQUIRE(inspector::changedHere(car));

    inspector::InField name("name");
    REQUIRE(inspector::changedHere(car.name));
}

TEST_CASE("A field under a field is found by the path it is drawn at", "[SavedInScope]")
{
    saved::CarData car;
    SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));

    car.wheels.grip = 0.5f;

    inspector::InField wheels("wheels");
    REQUIRE(inspector::changedHere(car.wheels));

    inspector::InField count("count");
    REQUIRE_FALSE(inspector::changedHere(car.wheels.count));
}

TEST_CASE(
    "A field the saved copy said nothing about is its default until it is edited",
    "[SavedInScope]")
{
    saved::CarData car;
    SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));

    inspector::InField wheels("wheels");
    inspector::InField grip("grip");
    REQUIRE_FALSE(inspector::changedHere(car.wheels.grip));

    car.wheels.grip = 0.5f;
    REQUIRE(inspector::changedHere(car.wheels.grip));
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
