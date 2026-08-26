#pragma once

#include <string>
#include <sol/sol.hpp>

class Game;
class Camera2D;
class TileMap;
class Player;
class ScreenTransition;

class LuaScriptSystem
{
public:
    struct WaitingCoroutine
    {
        sol::thread thread;
        sol::function co;
        float remainingTime;
    };

    explicit LuaScriptSystem(const std::string &scriptPath = "../../assets/scripts/game_logic.lua");
    void update(float deltaTime);
    void bindGameObjects(
        Game *game,
        Camera2D *camera,
        ScreenTransition *screenTransition);
    void triggerLevelComplete();
    void triggerDeath();
    void bindTileMap(TileMap *tileMap);
    sol::state &getLua();
    const std::vector<WaitingCoroutine> &getWaitingCoroutines() const;
    void triggerWallJump();
    void triggerDoubleJump();
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
    sol::function
        onDeath,
        onLevelComplete,
        onWallJump,
        onDoubleJump,
        onDash,
        onFallFromHeight,
        onHitCeiling,
        onWallSliding,
        onGameLoaded;
    std::vector<WaitingCoroutine> waitingCoroutines;
};