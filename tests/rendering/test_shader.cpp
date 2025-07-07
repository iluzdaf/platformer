#ifndef SKIP_OPENGL_TESTS
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "rendering/shader.hpp"
#include "rendering/shader_data.hpp"

TEST_CASE("Shader is valid", "[Shader]")
{
    ShaderData shaderData;
    shaderData.vertexPath = "../../assets/shaders/sprite.vs";
    shaderData.fragmentPath = "../../assets/shaders/sprite.fs";
    REQUIRE_NOTHROW(Shader(shaderData));
}

TEST_CASE("Shader does not exist", "[Shader]")
{
    ShaderData shaderData;
    shaderData.vertexPath = "../../assets/shaders/does_not_exist.vs";
    shaderData.fragmentPath = "../../assets/shaders/does_not_exist.fs";
    REQUIRE_THROWS_WITH(
        Shader(shaderData),
        "Vertex shader code is empty");
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

    const std::string brokenFragment = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0); // this is actually valid
        }
    )";

    ShaderData shaderData;
    shaderData.vertexCode = brokenVertex;
    shaderData.fragmentCode = brokenFragment;
    REQUIRE_THROWS(Shader(shaderData));
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

    ShaderData shaderData;
    shaderData.vertexCode = vertexShaderCode;
    shaderData.fragmentCode = fragmentShaderCode;
    REQUIRE_THROWS(Shader(shaderData));
}
#endif // SKIP_OPENGL_TESTS