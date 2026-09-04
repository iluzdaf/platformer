#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <string>
#include "rendering/shader.hpp"
#include "test_helpers/asset_path.hpp"

TEST_CASE("A shader file is read whole", "[Shader]")
{
    std::string code = readShaderFile(assetPath("shaders/sprite.vs"));

    REQUIRE_THAT(code, Catch::Matchers::ContainsSubstring("#version"));
    REQUIRE_THAT(code, Catch::Matchers::ContainsSubstring("gl_Position"));
}

TEST_CASE("A shader file that is not there says which", "[Shader]")
{
    REQUIRE_THROWS_WITH(
        readShaderFile(assetPath("shaders/does_not_exist.vs")),
        Catch::Matchers::ContainsSubstring("does_not_exist.vs"));
}
