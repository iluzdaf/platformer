#pragma once

#include "assets/asset_paths.hpp"
#include <optional>
#include <string>
#include <string_view>
#include <utility>
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
    template <typename... Args> void emit(std::string_view hook, Args &&...args)
    {
        sol::object handler = lua[hook];
        if (!handler.is<sol::function>())
            return;

        sol::protected_function call = handler.as<sol::protected_function>();
        settle(call(std::forward<Args>(args)...), hook);
    }
    void bindLevel(const Level *level);
    sol::state &getLua();
    void loadScripts();
    void bindPlayer(Player *player);

private:
    std::string scriptPath;
    sol::state lua;
    std::optional<float> resume(sol::protected_function &co, std::string_view what);
    std::optional<float> settle(sol::protected_function_result result, std::string_view what);
    std::vector<WaitingCoroutine> waitingCoroutines;
};