#pragma once

#include "assets/asset_paths.hpp"
#include <optional>
#include <string>
#include <string_view>
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
        sol::protected_function co;
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
    void emit(std::string_view hook);
    void bindLevel(const Level *level);
    sol::state &getLua();
    void loadScripts();
    void bindPlayer(Player *player);

private:
    std::string scriptPath;
    sol::state lua;
    std::optional<float> resume(sol::protected_function &co, std::string_view what);
    std::vector<WaitingCoroutine> waitingCoroutines;
};