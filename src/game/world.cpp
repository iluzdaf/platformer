#include <memory>
#include <vector>
#include "game/level_data.hpp"
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "game/world.hpp"
#include "actor/observed.hpp"
#include "actor/actor_contact_state.hpp"
#include "game/noise.hpp"
#include "game/level_data_file.hpp"
#include "game/game_data.hpp"
#include "assets/asset_paths.hpp"
#include "game/level.hpp"
#include "pickups/pickup.hpp"
#include "actor/actor.hpp"
#include "tile_map/touching_tiles.hpp"
#include "npc/striking_player.hpp"
#include "npc/striking_npcs.hpp"
#include "npc/npc.hpp"
#include "player/player.hpp"
#include "input/intention_source.hpp"
#include "scripting/lua_script_system.hpp"

namespace
{
    constexpr std::string_view PlayerScript = "player";

    std::string scriptOf(const std::string &npcType)
    {
        return "npc:" + npcType;
    }
}

World::World(
    const GameData &gameData,
    const IntentionSource &intentionSource,
    LuaScriptSystem &luaScriptSystem)
    : gameData(gameData), intentionSource(intentionSource), luaScriptSystem(luaScriptSystem)
{
    if (!gameData.playerData.script.empty())
        luaScriptSystem.use(PlayerScript, assets::pathTo(gameData.playerData.script));

    for (const auto &[type, npcData] : gameData.npcData)
        if (!npcData.script.empty())
            luaScriptSystem.use(scriptOf(type), assets::pathTo(npcData.script));
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

    if (level)
        for (const std::unique_ptr<Npc> &npc : level->getNpcs())
            luaScriptSystem.forget(npc.get());

    for (const std::unique_ptr<Npc> &npc : built->getNpcs())
    {
        Npc *it = npc.get();
        it->onHurt.connect([this, it]
                           { luaScriptSystem.emitTo(scriptOf(it->type()), "onHurt", it, it); });
        it->onDeath.connect([this, it]
                            { luaScriptSystem.emitTo(scriptOf(it->type()), "onDied", it, it); });
        it->onCue.connect([this, it](const std::string &cue)
                          { luaScriptSystem.emitTo(scriptOf(it->type()), cue, it, it); });
    }

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
    auto hear = [this, who = player.get()](const auto &event, std::string_view hook)
    {
        event.connect([this, hook, who]
                      { luaScriptSystem.emitTo(PlayerScript, hook, nullptr, who); });
    };
    hear(player->onLevelComplete, "onLevelComplete");
    hear(player->onDeath, "onDeath");
    hear(player->onHurt, "onHurt");
    hear(player->onWallJump, "onWallJump");
    hear(player->onDash, "onDash");
    hear(player->onAttack, "onAttack");
    hear(player->onWallSliding, "onWallSliding");
    hear(player->onFallFromHeight, "onFallFromHeight");
    hear(player->onHitCeiling, "onHitCeiling");
    player->onCue.connect([this, who = player.get()](const std::string &cue)
                          { luaScriptSystem.emitTo(PlayerScript, cue, nullptr, who); });
    luaScriptSystem.bindPlayer(player.get());
}

void World::beginFrame()
{
    level->beginFrame();
    player->beginFrame();
}

void World::fixedUpdate(float deltaTime)
{
    level->fixedUpdate(deltaTime, player->feet(), heardThisTick);
    heardThisTick.clear();
    player->fixedUpdate(deltaTime, *level.get(), std::nullopt);
    const ActorContactState &contacts = player->observed().contacts;
    if (!contacts.wasOnGround && contacts.onGround && player->observed().previousVelocity.y > 0.0f)
        heardThisTick.push_back(Noise{std::string(LandingNoise), player->feet()});

    strikeNpcs(*player.get(), level->getNpcs());
    touchTiles(*player.get(), level->getTileMap());
    strikePlayer(*player.get(), level->getNpcs());

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

const std::vector<Noise> &World::noises() const
{
    return heardThisTick;
}
