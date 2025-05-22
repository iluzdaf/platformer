#pragma once
#include "texture2d.hpp"
#include "sprite_renderer.hpp"
#include "tile_map.hpp"
#include <glm/glm.hpp>

class TileMapRenderer
{
public:
    TileMapRenderer(const Texture2D &tileSet, const int tileSize, const SpriteRenderer &spriteRenderer);
    void draw(const TileMap &map, const glm::mat4 &projection);

private:
    Texture2D tileSet;
    int tileSize;
    SpriteRenderer spriteRenderer;
    int tilesPerRow;
};