#include <exception>
#include <iostream>
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
#include "rendering/sheet_textures.hpp"
#include "rendering/texture2d.hpp"
#include "tile_map/tile_map.hpp"
#include "game/game_data.hpp"
#include "actor/actor.hpp"
#include "pickups/pickup.hpp"
#include "actor/actor_state.hpp"
#include "assets/sheet.hpp"
#include "assets/asset_paths.hpp"

GameRenderer::GameRenderer()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    reloadShader(std::string(assets::TileSetVertexShader));
    reloadShader(std::string(assets::TransitionVertexShader));
}

void GameRenderer::warm(const GameData &gameData)
{
    warmTileSets(textures, gameData.tilePalettes);
    warmActorTextures(textures, gameData.playerData, gameData.npcData);
    warmPickupTextures(textures, gameData.pickupData);
    warmScoreIcon(textures, gameData.settings.scoreIcon);
}

void GameRenderer::warmTexture(const std::string &texturePath)
{
    try
    {
        textures.warm(texturePath);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
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
    const std::vector<Pickup> &pickups,
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

    for (const Pickup &pickup : pickups)
    {
        const Sheet &sheet = pickup.getSheet();
        const Texture2D &texture = textures.get(sheet.texture);
        auto [uvStart, uvEnd] = frameUvRangeIn(
            static_cast<int>(texture.getWidth()),
            static_cast<int>(texture.getHeight()),
            pickup.getCurrentFrame(),
            sheet.cellSize.x,
            sheet.cellSize.y);

        spriteRenderer.drawWithUV(
            *tileSetShader.get(),
            texture,
            projection,
            pickup.getPosition(),
            pickup.getSize(),
            uvStart,
            uvEnd);
    }

    for (const Actor *actor : actors)
    {
        const ActorState &actorState = actor->getState();
        const Sheet &sheet = actor->getSheet();
        const Texture2D &texture = textures.get(sheet.texture);
        auto [uvStart, uvEnd] = frameUvRangeIn(
            static_cast<int>(texture.getWidth()),
            static_cast<int>(texture.getHeight()),
            actorState.currentFrame,
            sheet.cellSize.x,
            sheet.cellSize.y);

        spriteRenderer.drawWithUV(
            *tileSetShader.get(),
            texture,
            projection,
            actor->getPosition(),
            actorState.size,
            uvStart,
            uvEnd,
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
