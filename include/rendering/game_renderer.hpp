#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/shader.hpp"
#include "rendering/sprite_renderer.hpp"
#include "rendering/texture_cache.hpp"
#include "tile_map/tile_palette.hpp"

class Actor;
class ScreenTransition;
class TileMap;

class GameRenderer
{
public:
    GameRenderer();

    void warm(const TilePalettes &tilePalettes);

    void reloadShader(const std::string &shaderPath);
    void reloadTexture(const std::string &texturePath);

    void draw(
        const glm::mat4 &projection,
        const TileMap &tileMap,
        const std::vector<Actor *> &actors) const;
    void draw(const ScreenTransition &screenTransition) const;

    const TextureCache &getTextures() const;

private:
    TextureCache textures;
    std::unique_ptr<Shader> tileSetShader, screenTransitionShader;
    SpriteRenderer spriteRenderer;

    static std::unique_ptr<Shader> loadShader(std::string_view vertex, std::string_view fragment);
};
