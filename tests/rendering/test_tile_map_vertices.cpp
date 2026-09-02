#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cstddef>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/texture2d.hpp"
#include "rendering/tile_map_vertices.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/asset_path.hpp"

namespace
{
    constexpr int Sheet = 112;
    constexpr int Cell = 16;

    std::vector<float> verticesOf(const TileMap &tileMap)
    {
        std::vector<float> vertices;
        appendTileMapVertices(vertices, tileMap, Sheet, Sheet);
        return vertices;
    }

    std::size_t floatsFor(int tiles)
    {
        return static_cast<std::size_t>(tiles) * VerticesPerTile * FloatsPerVertex;
    }
}

TEST_CASE("Every cell of the map contributes one quad", "[TileMapVertices]")
{
    TileMap tileMap = setupTileMap(7, 5, Cell);

    REQUIRE(verticesOf(tileMap).size() == floatsFor(7 * 5));
}

TEST_CASE("A quad stands where the tile map puts its tile", "[TileMapVertices]")
{
    TileMap tileMap = setupTileMap(3, 3, Cell);
    std::vector<float> vertices = verticesOf(tileMap);

    glm::vec2 topLeft = tileMap.topLeftOfTile(glm::ivec2(2, 1));
    std::size_t at = floatsFor(1 * 3 + 2);

    REQUIRE(vertices[at] == topLeft.x);
    REQUIRE(vertices[at + 1] == topLeft.y + Cell);
}

TEST_CASE("A quad spans exactly one tile", "[TileMapVertices]")
{
    TileMap tileMap = setupTileMap(2, 1, Cell);
    std::vector<float> vertices = verticesOf(tileMap);

    float leftMost = vertices[0], rightMost = vertices[0];
    for (std::size_t at = 0; at < floatsFor(1); at += FloatsPerVertex)
    {
        leftMost = std::min(leftMost, vertices[at]);
        rightMost = std::max(rightMost, vertices[at]);
    }

    REQUIRE(rightMost - leftMost == Cell);
}

TEST_CASE("A quad reads the cell of the sheet its tile names", "[TileMapVertices]")
{
    TileMap tileMap = setupTileMap(2, 1, Cell);
    tileMap.setTileIndex(glm::ivec2(1, 0), 9);
    std::vector<float> vertices = verticesOf(tileMap);

    auto [uvStart, uvEnd] = uvRangeIn(Sheet, Sheet, 9, Cell);
    std::size_t at = floatsFor(1);

    float lowest = vertices[at + 2], highest = vertices[at + 2];
    for (std::size_t corner = at; corner < at + floatsFor(1); corner += FloatsPerVertex)
    {
        lowest = std::min(lowest, vertices[corner + 2]);
        highest = std::max(highest, vertices[corner + 2]);
    }

    REQUIRE(lowest == uvStart.x);
    REQUIRE(highest == uvEnd.x);
}

TEST_CASE("An animated tile contributes the frame it is showing", "[TileMapVertices]")
{
    TileData animated;
    animated.animationData = {{{3, 8}, 0.5f}};
    TileMap tileMap = setupTileMap(1, 1, Cell, paletteOf({{0, animated}}));

    auto uvOf = [](const std::vector<float> &vertices) { return vertices[2]; };

    float first = uvOf(verticesOf(tileMap));
    tileMap.update(0.5f);
    float second = uvOf(verticesOf(tileMap));

    REQUIRE(first == uvRangeIn(Sheet, Sheet, 3, Cell).first.x);
    REQUIRE(second == uvRangeIn(Sheet, Sheet, 8, Cell).first.x);
}

#ifndef SKIP_OPENGL_TESTS

#include <array>
#include <cstdint>
#include <string>
#include <glad/glad.h>
#include "assets/asset_paths.hpp"
#include "rendering/shader.hpp"
#include "rendering/shader_data.hpp"
#include "rendering/tile_map_batch.hpp"

namespace
{
    constexpr int Coin = 42;
    constexpr int Stone = 12;

    struct OffscreenTarget
    {
        OffscreenTarget(int width, int height) : width(width), height(height)
        {
            glGenTextures(1, &colour);
            glBindTexture(GL_TEXTURE_2D, colour);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glGenFramebuffers(1, &frameBuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colour, 0);
            glViewport(0, 0, width, height);
        }

        ~OffscreenTarget()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &frameBuffer);
            glDeleteTextures(1, &colour);
        }

        OffscreenTarget(const OffscreenTarget &) = delete;
        OffscreenTarget &operator=(const OffscreenTarget &) = delete;

        std::array<std::uint8_t, 4> pixelAt(int x, int y) const
        {
            std::array<std::uint8_t, 4> pixel{};
            glReadPixels(x, height - 1 - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
            return pixel;
        }

        bool complete() const
        {
            return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        }

        int width, height;
        GLuint colour = 0, frameBuffer = 0;
    };
}

TEST_CASE("The map draws each tile in its own place", "[TileMapVertices][OpenGL]")
{
    TileMap tileMap = setupTileMap(2, 1, Cell, shippedPalettes().at("default"));
    tileMap.setTileIndex(glm::ivec2(0, 0), Coin);
    tileMap.setTileIndex(glm::ivec2(1, 0), Stone);

    Texture2D sheet(assetPath(std::string(assets::TileSetTexture)));
    ShaderData shaderData;
    shaderData.vertexPath = assetPath(std::string(assets::TileSetVertexShader));
    shaderData.fragmentPath = assetPath(std::string(assets::TileSetFragmentShader));
    Shader shader(shaderData);

    OffscreenTarget target(2 * Cell, Cell);
    REQUIRE(target.complete());

    glDisable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    TileMapBatch batch;
    batch.draw(
        shader, tileMap, glm::ortho(0.0f, 2.0f * Cell, static_cast<float>(Cell), 0.0f), sheet);

    std::array<std::uint8_t, 4> coin = target.pixelAt(Cell / 2, Cell / 2);
    std::array<std::uint8_t, 4> stone = target.pixelAt(Cell + Cell / 2, Cell / 2);

    REQUIRE(coin[0] > coin[2] + 40);
    REQUIRE(stone[0] < stone[2] + 12);
    REQUIRE(stone[2] < coin[2]);
}

#endif
