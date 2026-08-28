#pragma once

#include <glm/gtc/matrix_transform.hpp>

class SpriteRenderer;
class Shader;
class TileMap;
class Texture2D;

void drawTileMap(
    const SpriteRenderer &spriteRenderer,
    const TileMap &tileMap,
    const glm::mat4 &projection,
    const Shader &tileSetShader,
    const Texture2D &tileSet);
