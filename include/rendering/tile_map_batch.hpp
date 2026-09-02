#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

class Shader;
class TileMap;
class Texture2D;

class TileMapBatch
{
public:
    TileMapBatch();
    ~TileMapBatch();

    TileMapBatch(const TileMapBatch &) = delete;
    TileMapBatch &operator=(const TileMapBatch &) = delete;

    void draw(
        const Shader &tileSetShader,
        const TileMap &tileMap,
        const glm::mat4 &projection,
        const Texture2D &tileSet) const;

private:
    GLuint vertexArrayObject = 0, vertexBufferObject = 0;
    mutable std::vector<float> vertices;
    mutable GLsizeiptr uploaded = 0;
};
