#include <memory>
#include "game/level_data.hpp"
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "game/world.hpp"
#include "game/level_data_file.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "tile_map/tile_map.hpp"
#include "pickups/pickup.hpp"
#include "actor/actor.hpp"
#include "tile_map/touching_tiles.hpp"
#include "player/player.hpp"
#include "input/intention_source.hpp"
#include "scripting/lua_script_system.hpp"

World::World(
    GameData &gameData,
    const IntentionSource &intentionSource,
    LuaScriptSystem &luaScriptSystem)
    : gameData(gameData), intentionSource(intentionSource), luaScriptSystem(luaScriptSystem)
{
}

World::~World() = default;

void World::loadLevel(const std::string &levelPath)
{
    path = levelPath;
    rebuildFrom(readLevelData(levelPath));
}

void World::rebuildFrom(const LevelData &fromData)
{
    levelData = fromData;
    level = std::make_unique<Level>(
        levelData,
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);
    luaScriptSystem.bindLevel(level.get());
    levelData.tileMapData = level->getTileMap().toTileMapData();

    respawnPlayer();

    onLevelLoaded();
}

const LevelData &World::getLevelData() const
{
    return levelData;
}

void World::respawnPlayer()
{
    if (!level)
        throw std::runtime_error("Cannot spawn the player before the tile map");

    std::unique_ptr<Player> newPlayer =
        std::make_unique<Player>(gameData.playerData, intentionSource);
    player = std::move(newPlayer);
    player->setPosition(level->getPlayerStart() - player->getPhysicsBody().getBottomCenterOffset());
    player->onDeath.connect([this] { luaScriptSystem.triggerDeath(); });
    onLevelCompleteConnection = player->onLevelComplete.connect(
        [this]()
        {
            onLevelCompleteConnection.block();
            luaScriptSystem.triggerLevelComplete();
        });
    player->onWallJump.connect([this] { luaScriptSystem.triggerWallJump(); });
    player->onDash.connect([this] { luaScriptSystem.triggerDash(); });
    player->onWallSliding.connect([this] { luaScriptSystem.triggerWallSliding(); });
    player->onFallFromHeight.connect([this] { luaScriptSystem.triggerFallFromHeight(); });
    player->onHitCeiling.connect([this] { luaScriptSystem.triggerHitCeiling(); });
    luaScriptSystem.bindPlayer(player.get());
}

void World::preFixedUpdate()
{
    level->preFixedUpdate();
    player->preFixedUpdate();
}

void World::fixedUpdate(float deltaTime)
{
    level->fixedUpdate(deltaTime, player->getPhysicsBody().getAABB().bottomCenter());
    player->fixedUpdate(deltaTime, *level.get(), std::nullopt);

    touchTiles(*player.get(), level->getTileMap());

    for (const Pickup &taken : level->takePickupsTouching(player->getPhysicsBody().getAABB()))
        score.add(taken.getScoreDelta());
}

void World::postFixedUpdate()
{
    level->postFixedUpdate();
    player->postFixedUpdate();
}

void World::update(float deltaTime)
{
    level->update(deltaTime);
}

const std::string &World::getLevelPath() const
{
    return path;
}

const Level &World::getLevel() const
{
    return *level.get();
}

const Player &World::getPlayer() const
{
    return *player.get();
}

const Score &World::getScore() const
{
    return score;
}
