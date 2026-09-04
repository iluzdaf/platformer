#ifndef SKIP_OPENGL_TESTS
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "rendering/screen_transition.hpp"

using Catch::Approx;

TEST_CASE("A fade in goes from dark to clear over its duration", "[ScreenTransition]")
{
    ScreenTransition transition;

    transition.start(1.0f, true);
    REQUIRE(transition.isActive());
    REQUIRE(transition.getAlpha() == Approx(1.0f));

    transition.update(0.25f);
    REQUIRE(transition.getAlpha() == Approx(0.75f));

    transition.update(0.75f);
    REQUIRE(transition.getAlpha() == Approx(0.0f));
    REQUIRE_FALSE(transition.isActive());
}

TEST_CASE("A fade out goes from clear to dark", "[ScreenTransition]")
{
    ScreenTransition transition;

    transition.start(2.0f, false);
    REQUIRE(transition.getAlpha() == Approx(0.0f));

    transition.update(0.5f);
    REQUIRE(transition.getAlpha() == Approx(0.25f));
}

TEST_CASE("A transition that is over stays where it ended", "[ScreenTransition]")
{
    ScreenTransition transition;
    transition.start(1.0f, false);
    transition.update(5.0f);
    REQUIRE_FALSE(transition.isActive());

    transition.update(1.0f);

    REQUIRE(transition.getAlpha() == Approx(1.0f));
}

TEST_CASE("Nothing fades until it is asked to", "[ScreenTransition]")
{
    ScreenTransition transition;

    transition.update(1.0f);

    REQUIRE_FALSE(transition.isActive());
    REQUIRE(transition.getAlpha() == Approx(0.0f));
}
#endif // SKIP_OPENGL_TESTS
