#pragma once
#include <glm/glm.hpp>
#include "rendering/texture2d.hpp"
#include "rendering/sprite_renderer.hpp"
#include "game/tile_map/tile_map.hpp"

class TileMapRenderer
{
public:
    TileMapRenderer(const Texture2D &tileSet, const SpriteRenderer &spriteRenderer);
    void draw(const TileMap &map, const glm::mat4 &projection);

private:
    Texture2D tileSet;
    SpriteRenderer spriteRenderer;
};