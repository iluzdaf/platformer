#ifndef SKIP_OPENGL_TESTS
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "window/window.hpp"

TEST_CASE("A window of no size is refused", "[Window]")
{
    REQUIRE_THROWS_WITH(Window(0, 0, "nothing"), "Failed to create window");
}
#endif // SKIP_OPENGL_TESTS
