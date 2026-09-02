#include <string>
#include <catch2/catch_test_macros.hpp>
#include "cameras/camera2d_data.hpp"
#include "ui/saveable.hpp"

TEST_CASE("Saveable has nothing to save until a value changes", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    saveable.seen("camera", asJson(data));

    REQUIRE_FALSE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("Saveable notices a changed value", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    saveable.seen("camera", asJson(data));
    data.zoom += 1.0f;

    REQUIRE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("Saveable keeps its first baseline however often it sees a value", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    saveable.seen("camera", asJson(data));
    data.zoom += 1.0f;
    saveable.seen("camera", asJson(data));

    REQUIRE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("Saveable moves its baseline when a value is saved", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    saveable.seen("camera", asJson(data));
    data.zoom += 1.0f;
    saveable.saved("camera", asJson(data));

    REQUIRE_FALSE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("Saveable takes replaced values as the new baseline", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    saveable.seen("camera", asJson(data));
    data.zoom += 1.0f;
    saveable.valuesReplaced();
    saveable.seen("camera", asJson(data));

    REQUIRE_FALSE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("Saveable remembers the baseline a revert would go back to", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;
    std::string wasOnDisk = asJson(data);

    saveable.seen("camera", wasOnDisk);
    data.zoom += 1.0f;

    REQUIRE(saveable.lastSeen("camera") == wasOnDisk);
}

TEST_CASE("Saveable has nothing to say about a name it has never seen", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    REQUIRE_FALSE(saveable.unsaved("camera", asJson(data)));
    REQUIRE(saveable.lastSeen("camera").empty());
}

TEST_CASE("The first look is what everything after is compared against", "[Saveable]")
{
    Saveable saveable;

    REQUIRE_FALSE(saveable.unsavedSince("camera", "one"));
    REQUIRE(saveable.unsavedSince("camera", "two"));
    REQUIRE(saveable.unsavedSince("camera", "three"));
}

TEST_CASE("Saving moves what everything after is compared against", "[Saveable]")
{
    Saveable saveable;

    REQUIRE_FALSE(saveable.unsavedSince("camera", "one"));
    saveable.saved("camera", "two");

    REQUIRE_FALSE(saveable.unsavedSince("camera", "two"));
    REQUIRE(saveable.unsavedSince("camera", "one"));
}
