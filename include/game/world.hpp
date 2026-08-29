#pragma once

#include <memory>
#include <string>
#include <vector>
#include <signals.hpp>
#include "game/scoring_system.hpp"
#include "tile_map/tile_interaction_system.hpp"

class Actor;
class InputManager;
class Level;
class LuaScriptSystem;
class Npc;
class Player;
struct GameData;

class World
{
public:
    World(GameData &gameData, InputManager &inputManager, LuaScriptSystem &luaScriptSystem);
    ~World();

    void loadLevel(const std::string &levelPath);

    fteng::signal<void()> onLevelLoaded;
    void rebuildPlayer();

    void preFixedUpdate();
    void fixedUpdate(float deltaTime);
    void postFixedUpdate();
    void update(float deltaTime);

    bool isPlaying(const std::string &levelPath) const;
    std::string getLevelPath() const;

    Level &getLevel();
    const Player &getPlayer() const;
    const std::vector<std::unique_ptr<Npc>> &getNpcs() const;
    const std::vector<Actor *> &getActors() const;
    const ScoringSystem &getScoringSystem() const;

private:
    void rebuildLevel(const std::string &levelPath);
    void rebuildNpcs();
    void refreshActors();

    GameData &gameData;
    InputManager &inputManager;
    LuaScriptSystem &luaScriptSystem;

    std::unique_ptr<Level> level;
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Npc>> npcs;
    std::vector<Actor *> actors;
    TileInteractionSystem tileInteractionSystem;
    ScoringSystem scoringSystem;
    fteng::connection onLevelCompleteConnection;
};
