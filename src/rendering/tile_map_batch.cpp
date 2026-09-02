#include <cstddef>
#include <vector>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/tile_map_batch.hpp"
#include "rendering/tile_map_vertices.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture2d.hpp"

namespace
{
    constexpr auto Stride = static_cast<GLsizei>(FloatsPerVertex * sizeof(float));
}

TileMapBatch::TileMapBatch()
{
    glGenVertexArrays(1, &vertexArrayObject);
    glGenBuffers(1, &vertexBufferObject);
    glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, Stride, nullptr);
    glEnableVertexAttribArray(1);
    // NOLINTNEXTLINE(performance-no-int-to-ptr) glVertexAttribPointer takes an offset as a pointer
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, Stride, (void *)(2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

TileMapBatch::~TileMapBatch()
{
    if (vertexBufferObject != 0)
        glDeleteBuffers(1, &vertexBufferObject);

    if (vertexArrayObject != 0)
        glDeleteVertexArrays(1, &vertexArrayObject);
}

void TileMapBatch::draw(
    const Shader &tileSetShader,
    const TileMap &tileMap,
    const glm::mat4 &projection,
    const Texture2D &tileSet) const
{
    vertices.clear();
    appendTileMapVertices(
        vertices,
        tileMap,
        static_cast<int>(tileSet.getWidth()),
        static_cast<int>(tileSet.getHeight()));

    if (vertices.empty())
        return;

    tileSetShader.use();
    tileSetShader.setMat4("projection", projection);
    tileSetShader.setMat4("model", glm::mat4(1.0f));
    tileSetShader.setVec2("uvStart", glm::vec2(0.0f, 0.0f));
    tileSetShader.setVec2("uvEnd", glm::vec2(1.0f, 1.0f));
    tileSetShader.setInt("image", 0);

    glActiveTexture(GL_TEXTURE0);
    tileSet.bind();

    glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);

    auto wanted = static_cast<GLsizeiptr>(vertices.size() * sizeof(float));
    if (wanted > uploaded)
    {
        glBufferData(GL_ARRAY_BUFFER, wanted, vertices.data(), GL_DYNAMIC_DRAW);
        uploaded = wanted;
    }
    else
        glBufferSubData(GL_ARRAY_BUFFER, 0, wanted, vertices.data());

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / FloatsPerVertex));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
