#pragma once

#include <memory>
#include <string>
#include <signals.hpp>
#include "game/score.hpp"

class IntentionSource;
class Level;
class LuaScriptSystem;
class Player;
struct GameData;

class World
{
public:
    World(
        GameData &gameData,
        const IntentionSource &intentionSource,
        LuaScriptSystem &luaScriptSystem);
    ~World();

    void loadLevel(const std::string &levelPath);

    fteng::signal<void()> onLevelLoaded;
    void respawnPlayer();

    void preFixedUpdate();
    void fixedUpdate(float deltaTime);
    void postFixedUpdate();
    void update(float deltaTime);

    bool isPlaying(const std::string &levelPath) const;
    const std::string &getLevelPath() const;

    Level &getLevel();
    const Player &getPlayer() const;
    const Score &getScore() const;

    void rebuildNpcs();

private:
    void rebuildLevel(const std::string &levelPath);

    std::string path;

    GameData &gameData;
    const IntentionSource &intentionSource;
    LuaScriptSystem &luaScriptSystem;

    std::unique_ptr<Level> level;
    std::unique_ptr<Player> player;
    Score score;
    fteng::connection onLevelCompleteConnection;
};
