#ifndef SKIP_OPENGL_TESTS
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <string>
#include "rendering/shader.hpp"
#include "rendering/shader_data.hpp"
#include "test_helpers/asset_path.hpp"

namespace
{
    const std::string ValidVertex = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";

    const std::string ValidFragment = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";
}

TEST_CASE("Shader is valid", "[Shader]")
{
    ShaderData shaderData{
        readShaderFile(assetPath("shaders/sprite.vs")),
        readShaderFile(assetPath("shaders/sprite.fs"))};
    REQUIRE_NOTHROW(Shader(shaderData));
}

TEST_CASE("A shader with no code is refused", "[Shader]")
{
    REQUIRE_THROWS_WITH(Shader(ShaderData{"", ValidFragment}), "Vertex shader code is empty");
    REQUIRE_THROWS_WITH(Shader(ShaderData{ValidVertex, ""}), "Fragment shader code is empty");
}

TEST_CASE("Shader is broken", "[Shader]")
{
    const std::string brokenVertex = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 projection  // <- missing semicolon!
        void main() {
            gl_Position = projection * vec4(aPos, 1.0);
        }
    )";

    REQUIRE_THROWS_WITH(
        Shader(ShaderData{brokenVertex, ValidFragment}),
        Catch::Matchers::ContainsSubstring("Vertex shader compilation failed"));
}

TEST_CASE("A fragment shader that does not compile is refused", "[Shader]")
{
    const std::string brokenFragment = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0)  // <- missing semicolon!
        }
    )";

    REQUIRE_THROWS_WITH(
        Shader(ShaderData{ValidVertex, brokenFragment}),
        Catch::Matchers::ContainsSubstring("Fragment shader compilation failed"));
}

TEST_CASE("Shader fails to link", "[Shader]")
{
    const std::string vertexShaderCode = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;

        out vec3 fragData; // mismatched with fragment input

        void main() {
            fragData = aPos;
            gl_Position = vec4(aPos, 1.0);
        }
    )";

    const std::string fragmentShaderCode = R"(
        #version 330 core
        in vec2 fragData;  // mismatched: vertex sends vec3
        out vec4 FragColor;

        void main() {
            FragColor = vec4(fragData, 0.0, 1.0);
        }
    )";

    REQUIRE_THROWS_WITH(
        Shader(ShaderData{vertexShaderCode, fragmentShaderCode}),
        Catch::Matchers::ContainsSubstring("Shader program linking failed"));
}
#endif // SKIP_OPENGL_TESTS
