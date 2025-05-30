#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/texture2d.hpp"
#include "rendering/sprite_renderer.hpp"
class TileMap;

class TileMapRenderer
{
public:
    TileMapRenderer(const Texture2D &tileSet, const SpriteRenderer &spriteRenderer);
    void draw(const TileMap &map, const glm::mat4 &projection);

private:
    Texture2D tileSet;
    SpriteRenderer spriteRenderer;
};