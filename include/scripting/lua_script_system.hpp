#pragma once

#include "assets/asset_paths.hpp"
#include <string>
#include <sol/sol.hpp>
#include <vector>

class Playback;
class Camera2D;
class TileMap;
class Level;
class Player;
class ScreenTransition;
class World;

class LuaScriptSystem
{
public:
    struct WaitingCoroutine
    {
        sol::thread thread;
        sol::function co;
        float remainingTime;
    };

    explicit LuaScriptSystem(
        const std::string &scriptPath = assets::pathTo(assets::GameLogicScript));
    void update(float deltaTime);
    void bindGameObjects(
        Playback *playback,
        Camera2D *camera,
        ScreenTransition *screenTransition,
        World *world);
    void triggerLevelComplete();
    void triggerDeath();
    void bindLevel(Level *level);
    sol::state &getLua();
    const std::vector<WaitingCoroutine> &getWaitingCoroutines() const;
    void triggerWallJump();
    void triggerDash();
    void triggerFallFromHeight();
    void triggerHitCeiling();
    void triggerWallSliding();
    void triggerGameLoaded();
    void loadScripts();
    void bindPlayer(Player *player);

private:
    std::string scriptPath;
    sol::state lua;
    sol::function onDeath, onLevelComplete, onWallJump, onDash, onFallFromHeight, onHitCeiling,
        onWallSliding, onGameLoaded;
    std::vector<WaitingCoroutine> waitingCoroutines;
};