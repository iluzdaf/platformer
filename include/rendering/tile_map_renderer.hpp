#pragma once
#include <glm/gtc/matrix_transform.hpp>
class SpriteRenderer;
class Shader;
class TileMap;
class Texture2D;

class TileMapRenderer
{
public:
    void draw(
        const SpriteRenderer &spriteRenderer,
        const TileMap &map,
        const glm::mat4 &projection,
        const Shader &tileSetShader,
        const Texture2D &tileSet);
};
