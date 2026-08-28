#include <catch2/catch_test_macros.hpp>
#include <type_traits>
#include "rendering/screen_transition.hpp"
#include "rendering/shader.hpp"
#include "rendering/sprite_renderer.hpp"
#include "rendering/texture2d.hpp"

static_assert(!std::is_copy_constructible_v<SpriteRenderer>);
static_assert(!std::is_copy_assignable_v<SpriteRenderer>);
static_assert(!std::is_copy_constructible_v<Shader>);
static_assert(!std::is_copy_assignable_v<Shader>);
static_assert(!std::is_copy_constructible_v<Texture2D>);
static_assert(!std::is_copy_assignable_v<Texture2D>);
static_assert(!std::is_copy_constructible_v<ScreenTransition>);
static_assert(!std::is_copy_assignable_v<ScreenTransition>);

TEST_CASE("Nothing that frees a gl name can be copied into a second owner", "[Rendering]")
{
    SUCCEED("the static asserts above are the test");
}
