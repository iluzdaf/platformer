#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/sprite_renderer.hpp"
class TileMap;
class Texture2D;

class TileMapRenderer
{
public:
    TileMapRenderer(const SpriteRenderer &spriteRenderer);
    void draw(
        const TileMap &map,
        const glm::mat4 &projection,
        const Shader &tileSetShader,
        const Texture2D &tileSet);

private:
    SpriteRenderer spriteRenderer;
};