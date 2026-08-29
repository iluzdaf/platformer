#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/world.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "actor/actor.hpp"
#include "npc/npc.hpp"
#include "npc/npc_spawn_data.hpp"
#include "player/player.hpp"
#include "input/input_manager.hpp"
#include "scripting/lua_script_system.hpp"

World::World(GameData &gameData, InputManager &inputManager, LuaScriptSystem &luaScriptSystem)
    : gameData(gameData), inputManager(inputManager), luaScriptSystem(luaScriptSystem)
{
}

World::~World() = default;

void World::loadLevel(const std::string &levelPath)
{
    rebuildLevel(levelPath);

    rebuildPlayer();

    rebuildNpcs();

    onLevelLoaded();
}

void World::rebuildLevel(const std::string &levelPath)
{
    std::unique_ptr<Level> newLevel = std::make_unique<Level>(
        levelPath, gameData.tilePalettes, gameData.playerData, gameData.npcData);
    level = std::move(newLevel);
    luaScriptSystem.bindLevel(level.get());
}

void World::rebuildPlayer()
{
    if (!level)
        throw std::runtime_error("Cannot rebuild the player before the tile map");

    std::unique_ptr<Player> newPlayer = std::make_unique<Player>(gameData.playerData, inputManager);
    player = std::move(newPlayer);
    player->setPosition(
        level->getTileMap().tileToBottomCenterPosition(level->getPlayerStartTile()) -
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
        auto it = gameData.npcData.find(spawn.type);

        std::unique_ptr<Npc> newNpc = std::make_unique<Npc>(it->second, level->patrolFor(spawn));
        newNpc->setPosition(
            level->getTileMap().tileToBottomCenterPosition(spawn.tilePosition) -
            newNpc->getPhysicsBody().getBottomCenterOffset());
        npcs.push_back(std::move(newNpc));
    }

    refreshActors();
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

    tileInteractionSystem.fixedUpdate(*player.get(), level->getTileMap());
}

void World::postFixedUpdate()
{
    for (Actor *actor : actors)
        actor->postFixedUpdate();
}

void World::update(float deltaTime)
{
    level->getTileMap().update(deltaTime);
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
