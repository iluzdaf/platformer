#pragma once

#include "assets/asset_paths.hpp"
#include <map>
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
        const void *startedBy = nullptr;
    };

    struct NamedScript
    {
        std::string path;
        sol::environment environment;
        sol::table handlers;
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
    void use(std::string_view name, const std::string &path);
    template <typename... Args>
    void emitTo(std::string_view name, std::string_view hook, const void *owner, Args &&...args)
    {
        auto found = scripts.find(std::string(name));
        if (found == scripts.end() || !found->second.handlers.valid())
            return;

        sol::object handler = found->second.handlers[hook];
        if (!handler.is<sol::function>())
            return;

        startedBy = owner;
        settle(handler.as<sol::protected_function>()(std::forward<Args>(args)...), hook);
        startedBy = nullptr;
    }
    template <typename... Args>
    sol::object callState(
        std::string_view name,
        const void *owner,
        const std::string &state,
        std::string_view hook,
        Args &&...args)
    {
        sol::object handler = stateHook(name, state, hook);
        if (!handler.is<sol::function>())
            return sol::make_object(lua, sol::lua_nil);

        startedBy = owner;
        sol::protected_function_result result = handler.as<sol::protected_function>()(
            selfOf(owner, state), std::forward<Args>(args)...);
        startedBy = nullptr;
        if (!result.valid())
        {
            settle(std::move(result), state + "." + std::string(hook));
            return sol::make_object(lua, sol::lua_nil);
        }

        sol::object wanted = result;
        return wanted;
    }
    void startStateAfresh(const void *owner, const std::string &state);
    void forget(const void *owner);
    void bindLevel(const Level *level);
    sol::state &getLua();
    void loadScripts();
    void bindPlayer(Player *player);

private:
    std::string scriptPath;
    sol::state lua;
    std::map<std::string, NamedScript> scripts;
    std::map<std::pair<const void *, std::string>, sol::table> stateSelves;
    const void *startedBy = nullptr;
    void reload(NamedScript &script, std::string_view name);
    sol::object stateHook(std::string_view name, const std::string &state, std::string_view hook);
    sol::table selfOf(const void *owner, const std::string &state);
    std::optional<float> resume(sol::protected_function &co, std::string_view what);
    std::optional<float> settle(sol::protected_function_result result, std::string_view what);
    std::vector<WaitingCoroutine> waitingCoroutines;
};