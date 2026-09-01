#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/game_renderer.hpp"
#include "rendering/screen_transition.hpp"
#include "rendering/shader_data.hpp"
#include "rendering/tile_map_drawing.hpp"
#include "rendering/tile_set_textures.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette.hpp"
#include "actor/actor.hpp"
#include "actor/actor_state.hpp"
#include "assets/asset_paths.hpp"

GameRenderer::GameRenderer()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    textures.warm(std::string(assets::PlayerTexture));
    reloadShader(std::string(assets::TileSetVertexShader));
    reloadShader(std::string(assets::TransitionVertexShader));
}

void GameRenderer::warm(const TilePalettes &tilePalettes)
{
    warmTileSets(textures, tilePalettes);
}

std::unique_ptr<Shader> GameRenderer::loadShader(std::string_view vertex, std::string_view fragment)
{
    ShaderData shaderData;
    shaderData.vertexPath = assets::pathTo(vertex);
    shaderData.fragmentPath = assets::pathTo(fragment);

    return std::make_unique<Shader>(shaderData);
}

void GameRenderer::reloadShader(const std::string &shaderPath)
{
    if (shaderPath == assets::TileSetVertexShader || shaderPath == assets::TileSetFragmentShader)
        tileSetShader = loadShader(assets::TileSetVertexShader, assets::TileSetFragmentShader);
    else if (
        shaderPath == assets::TransitionVertexShader ||
        shaderPath == assets::TransitionFragmentShader)
        screenTransitionShader =
            loadShader(assets::TransitionVertexShader, assets::TransitionFragmentShader);
}

void GameRenderer::reloadTexture(const std::string &texturePath)
{
    textures.reload(texturePath);
}

void GameRenderer::draw(
    const glm::mat4 &projection,
    const TileMap &tileMap,
    const std::vector<Actor *> &actors) const
{
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawTileMap(
        spriteRenderer,
        tileMap,
        projection,
        *tileSetShader.get(),
        textures.get(tileMap.getTileSet().texture));

    for (const Actor *actor : actors)
    {
        const ActorState &actorState = actor->getState();
        spriteRenderer.drawWithUV(
            *tileSetShader.get(),
            textures.get(std::string(assets::PlayerTexture)),
            projection,
            actor->getPosition(),
            actorState.size,
            actorState.currentAnimationUVStart,
            actorState.currentAnimationUVEnd,
            actorState.facingLeft);
    }
}

void GameRenderer::draw(const ScreenTransition &screenTransition) const
{
    screenTransition.draw(*screenTransitionShader.get());
}

const TextureCache &GameRenderer::getTextures() const
{
    return textures;
}
