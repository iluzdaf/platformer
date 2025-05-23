#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "rendering/shader.hpp"

TEST_CASE("Shader is valid", "[shader]")
{
    Shader shader;
    shader.initByPath("../shaders/sprite.vs", "../shaders/sprite.fs");
    REQUIRE(shader.valid());
}

TEST_CASE("Shader does not exist", "[shader]")
{
    Shader shader;
    REQUIRE_THROWS_WITH(
        shader.initByPath("../shaders/does_not_exist.vs", "../shaders/does_not_exist.fs"),
        "Vertex shader code is empty");
}

TEST_CASE("Shader is broken", "[shader]")
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

    Shader shader;
    shader.initByCode(brokenVertex, brokenFragment);
    REQUIRE_FALSE(shader.valid());
}

TEST_CASE("Shader fails to link", "[shader]")
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

    Shader shader;
    shader.initByCode(vertexShaderCode, fragmentShaderCode);
    REQUIRE_FALSE(shader.valid());
}

TEST_CASE("Shader can only initialize once", "[shader]")
{
    Shader shader;
    shader.initByPath("../shaders/sprite.vs", "../shaders/sprite.fs");
    REQUIRE_THROWS_WITH(
        shader.initByPath("../shaders/sprite.vs", "../shaders/sprite.fs"),
        "Shader is already initialized");
}