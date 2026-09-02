#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/world.hpp"
#include "game/game_data.hpp"
#include "game/catalogue.hpp"
#include "game/level.hpp"
#include "actor/actor.hpp"
#include "npc/npc.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "pickups/pickup_data.hpp"
#include "pickups/collecting.hpp"
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
    rebuildLevel(levelPath);

    respawnPlayer();

    rebuildNpcs();

    rebuildPickups();

    onLevelLoaded();
}

void World::rebuildLevel(const std::string &levelPath)
{
    std::unique_ptr<Level> newLevel = std::make_unique<Level>(
        levelPath, gameData.tilePalettes, gameData.playerData, gameData.npcData);
    level = std::move(newLevel);
    luaScriptSystem.bindLevel(level.get());
}

void World::respawnPlayer()
{
    if (!level)
        throw std::runtime_error("Cannot spawn the player before the tile map");

    std::unique_ptr<Player> newPlayer =
        std::make_unique<Player>(gameData.playerData, intentionSource);
    player = std::move(newPlayer);
    player->setPosition(
        level->getTileMap().feetOnTile(level->getPlayerStartTile()) -
        player->getPhysicsBody().getBottomCenterOffset());
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
    player->onPickup.connect([this](int scoreDelta) { scoringSystem.addScore(scoreDelta); });
    luaScriptSystem.bindPlayer(player.get());

    refreshActors();
}

void World::rebuildNpcs()
{
    npcs.clear();

    for (const NpcSpawnData &spawn : level->getNpcs())
    {
        std::unique_ptr<Npc> newNpc = std::make_unique<Npc>(
            oneNamed(gameData.npcData, "npc", spawn.type), level->patrolFor(spawn));
        newNpc->setPosition(
            level->getTileMap().feetOnTile(spawn.tilePosition) -
            newNpc->getPhysicsBody().getBottomCenterOffset());
        npcs.push_back(std::move(newNpc));
    }

    refreshActors();
}

void World::rebuildPickups()
{
    pickups.clear();

    for (const PickupSpawnData &spawn : level->getPickups())
    {
        const PickupData &kind = oneNamed(gameData.pickupData, "pickup", spawn.type);

        pickups.push_back(
            Pickup(kind, level->getTileMap().middleOfTile(spawn.tilePosition) - kind.size * 0.5f));
    }
}

const std::vector<Pickup> &World::getPickups() const
{
    return pickups;
}

void World::refreshActors()
{
    actors.clear();

    for (auto &npc : npcs)
        actors.push_back(npc.get());

    if (player)
        actors.push_back(player.get());
}

void World::preFixedUpdate()
{
    for (Actor *actor : actors)
        actor->preFixedUpdate();
}

void World::fixedUpdate(float deltaTime)
{
    std::optional<glm::vec2> playerPosition;
    if (player)
        playerPosition = player->getPhysicsBody().getAABB().bottomCenter();

    for (Actor *actor : actors)
        actor->fixedUpdate(
            deltaTime, *level.get(), actor == player.get() ? std::nullopt : playerPosition);

    touchTiles(*player.get(), level->getTileMap());

    for (const Pickup &taken : takeWhatTouches(pickups, player->getPhysicsBody().getAABB()))
        player->onPickup(taken.getScoreDelta());
}

void World::postFixedUpdate()
{
    for (Actor *actor : actors)
        actor->postFixedUpdate();
}

void World::update(float deltaTime)
{
    level->getTileMap().update(deltaTime);

    for (Pickup &pickup : pickups)
        pickup.update(deltaTime);
}

bool World::isPlaying(const std::string &levelPath) const
{
    return level && level->getPath() == levelPath;
}

std::string World::getLevelPath() const
{
    return level ? level->getPath() : std::string();
}

Level &World::getLevel()
{
    return *level.get();
}

const Player &World::getPlayer() const
{
    return *player.get();
}

const std::vector<std::unique_ptr<Npc>> &World::getNpcs() const
{
    return npcs;
}

const std::vector<Actor *> &World::getActors() const
{
    return actors;
}

const ScoringSystem &World::getScoringSystem() const
{
    return scoringSystem;
}
