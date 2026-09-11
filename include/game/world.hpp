#pragma once

#include <memory>
#include <vector>
#include "game/noise.hpp"
#include <string>
#include "events/event.hpp"
#include "game/score.hpp"
#include "game/level_data.hpp"
#include "tile_map/tile_map_data.hpp"

class IntentionSource;
class Level;
class LuaScriptSystem;
class Player;
struct GameData;

class World
{
public:
    World(
        const GameData &gameData,
        const IntentionSource &intentionSource,
        LuaScriptSystem &luaScriptSystem);
    ~World();

    void loadLevel(const std::string &levelPath);
    void playLevel(const std::string &levelPath, const LevelData &fromData);
    void tilesChanged(const TileMapData &tileMapData);
    void rebuildFrom(
        const LevelData &fromData,
        const glm::vec2 &movingThePlayerBy = glm::vec2(0.0f));

    Event<World> onLevelBuilt;
    void respawnPlayer();
    void castChanged();

    void beginFrame();
    void fixedUpdate(float deltaTime);
    void postFixedUpdate();
    void update(float deltaTime);

    const std::string &getLevelPath() const;
    const LevelData &getLevelData() const;

    const Level &getLevel() const;
    const Player &getPlayer() const;
    const std::vector<Noise> &noises() const;
    Player &getPlayer();
    const Score &getScore() const;

private:
    std::string path;
    LevelData levelData;

    const GameData &gameData;
    const IntentionSource &intentionSource;
    LuaScriptSystem &luaScriptSystem;

    std::unique_ptr<Level> level;
    std::vector<Noise> heardThisTick;
    std::unique_ptr<Player> player;
    Score score;

    void build(const LevelData &fromData, const glm::vec2 &movingThePlayerBy);
    void makePlayerAt(glm::vec2 feet);
};
