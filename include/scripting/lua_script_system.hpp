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
        TileMap *tileMap,
        Player *player,
        ScreenTransition *screenTransition);
    void triggerLevelComplete();
    void triggerDeath();
    void rebindTileMap(TileMap *tileMap);
    sol::state &getLua();
    const std::vector<WaitingCoroutine> &getWaitingCoroutines() const;
    void triggerWallJump();
    void triggerDoubleJump();
    void triggerDash();

private:
    sol::state lua;
    sol::function onDeath;
    sol::function onLevelComplete;
    std::vector<WaitingCoroutine> waitingCoroutines;
    sol::function onWallJump;
    sol::function onDoubleJump;
    sol::function onDash;
};