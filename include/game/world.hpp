#pragma once

#include <memory>
#include <string>
#include <signals.hpp>
#include "game/score.hpp"
#include "game/level_data.hpp"

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
    void rebuildFrom(
        const LevelData &fromData,
        const glm::vec2 &movingThePlayerBy = glm::vec2(0.0f));

    fteng::signal<void()> onLevelBuilt;
    void respawnPlayer();

    void beginFrame();
    void fixedUpdate(float deltaTime);
    void postFixedUpdate();
    void update(float deltaTime);

    const std::string &getLevelPath() const;
    const LevelData &getLevelData() const;

    const Level &getLevel() const;
    const Player &getPlayer() const;
    Player &getPlayer();
    const Score &getScore() const;

private:
    std::string path;
    LevelData levelData;

    const GameData &gameData;
    const IntentionSource &intentionSource;
    LuaScriptSystem &luaScriptSystem;

    std::unique_ptr<Level> level;
    std::unique_ptr<Player> player;
    Score score;
};
