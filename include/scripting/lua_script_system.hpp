#pragma once
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
    LuaScriptSystem();
    void update(float deltaTime);
    void bindGameObjects(
        Game *game,
        Camera2D *camera,
        Player *player,
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
    void loadScripts();

private:
    sol::state lua;
    sol::function
        onDeath,
        onLevelComplete,
        onWallJump,
        onDoubleJump,
        onDash,
        onFallFromHeight,
        onHitCeiling,
        onWallSliding;
    std::vector<WaitingCoroutine> waitingCoroutines;
};