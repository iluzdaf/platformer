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
    void triggerRespawn();
    void rebindTileMap(TileMap *tileMap);
    sol::state &getLua();
    const std::vector<WaitingCoroutine> &getWaitingCoroutines() const;

private:
    sol::state lua;
    sol::function onRespawn;
    sol::function onLevelComplete;
    std::vector<WaitingCoroutine> waitingCoroutines;
};