#include <memory>
#include "game/level_data.hpp"
#include <optional>
#include <stdexcept>
#include <string>
#include <signals.hpp>
#include <string_view>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "game/world.hpp"
#include "game/level_data_file.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "pickups/pickup.hpp"
#include "actor/actor.hpp"
#include "tile_map/touching_tiles.hpp"
#include "player/player.hpp"
#include "input/intention_source.hpp"
#include "scripting/lua_script_system.hpp"

World::World(
    const GameData &gameData,
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
    respawnPlayer();
}

void World::rebuildFrom(const LevelData &fromData, const glm::vec2 &movingThePlayerBy)
{
    std::unique_ptr<Level> built = std::make_unique<Level>(
        fromData,
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData);

    levelData = fromData;
    level = std::move(built);
    luaScriptSystem.bindLevel(level.get());
    if (player)
        player->standAt(player->feet() + movingThePlayerBy);

    onLevelBuilt();
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
    player->standAt(level->getPlayerStart());
    const std::pair<fteng::signal<void()> &, std::string_view> hooks[] = {
        {player->onLevelComplete, "onLevelComplete"},
        {player->onDeath, "onDeath"},
        {player->onHurt, "onHurt"},
        {player->onWallJump, "onWallJump"},
        {player->onDash, "onDash"},
        {player->onWallSliding, "onWallSliding"},
        {player->onFallFromHeight, "onFallFromHeight"},
        {player->onHitCeiling, "onHitCeiling"}};
    for (auto &[signal, hook] : hooks)
        signal.connect([this, hook] { luaScriptSystem.emit(hook); });
    luaScriptSystem.bindPlayer(player.get());
}

void World::preFixedUpdate()
{
    level->preFixedUpdate();
    player->preFixedUpdate();
}

void World::fixedUpdate(float deltaTime)
{
    level->fixedUpdate(deltaTime, player->feet());
    player->fixedUpdate(deltaTime, *level.get(), std::nullopt);

    touchTiles(*player.get(), level->getTileMap());

    for (const Pickup &taken : level->takePickupsTouching(player->body().touchBox()))
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

Player &World::getPlayer()
{
    return *player;
}

const Score &World::getScore() const
{
    return score;
}
