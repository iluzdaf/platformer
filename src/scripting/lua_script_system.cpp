#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <optional>
#include <iostream>
#include <string_view>
#include <vector>
#include "scripting/lua_script_system.hpp"
#include "game/world.hpp"
#include "cameras/camera2d.hpp"
#include "rendering/screen_transition.hpp"
#include "player/player.hpp"
#include "npc/npc.hpp"
#include "actor/actor.hpp"
#include "game/playback.hpp"
#include "game/level.hpp"

namespace
{
    constexpr float WaitSlack = 1e-6f;
}

LuaScriptSystem::LuaScriptSystem(const std::string &scriptPath) : scriptPath(scriptPath)
{
    lua.open_libraries(
        sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string, sol::lib::coroutine);

    lua.new_usertype<glm::vec2>(
        "vec2",
        sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
        "x",
        &glm::vec2::x,
        "y",
        &glm::vec2::y);
    lua.new_usertype<World>(
        "World", "loadLevel", &World::loadLevel, "respawnPlayer", &World::respawnPlayer);
    lua.new_usertype<Playback>(
        "Playback", "pause", &Playback::pause, "play", &Playback::play, "step", &Playback::step);
    lua.new_usertype<Camera2D>("Camera", "startShake", &Camera2D::startShake);
    lua.new_usertype<Level>("Level", "getNextLevel", &Level::getNextLevel);
    lua.new_usertype<Actor>(
        "Actor", "feet", &Actor::feet, "standAt", &Actor::standAt, "alive", &Actor::alive);
    lua.new_usertype<Player>("Player", sol::base_classes, sol::bases<Actor>());
    lua.new_usertype<Npc>("Npc", "type", &Npc::type, sol::base_classes, sol::bases<Actor>());
    lua.new_usertype<ScreenTransition>("ScreenTransition", "start", &ScreenTransition::start);

    lua.set_function(
        "startCoroutine",
        [this](const sol::function &func)
        {
            sol::thread thread = sol::thread::create(lua.lua_state());
            sol::state_view threadState = thread.state();
            threadState["f"] = func;
            sol::protected_function co = threadState.load("return coroutine.wrap(f)")();
            if (std::optional<float> wait = resume(co, "a coroutine"))
                waitingCoroutines.push_back({thread, co, *wait});
        });

    loadScripts();
}

void LuaScriptSystem::update(float deltaTime)
{
    for (auto it = waitingCoroutines.begin(); it != waitingCoroutines.end();)
    {
        it->remainingTime -= deltaTime;
        if (it->remainingTime <= WaitSlack)
        {
            if (std::optional<float> wait = resume(it->co, "a coroutine"))
            {
                it->remainingTime = *wait;
                ++it;
            }
            else
                it = waitingCoroutines.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void LuaScriptSystem::bindGameObjects(
    Playback *playback,
    Camera2D *camera,
    ScreenTransition *screenTransition,
    World *world)
{
    lua["playback"] = playback;
    lua["camera"] = camera;
    lua["screenTransition"] = screenTransition;
    lua["world"] = world;
}

std::optional<float> LuaScriptSystem::resume(sol::protected_function &co, std::string_view what)
{
    return settle(co(), what);
}

std::optional<float> LuaScriptSystem::settle(
    sol::protected_function_result result,
    std::string_view what)
{
    if (!result.valid())
    {
        sol::error error = result;
        std::cerr << "Lua error in " << what << ": " << error.what() << '\n';
        return std::nullopt;
    }

    sol::object yielded = result;
    if (yielded.is<float>())
        return yielded.as<float>();

    return std::nullopt;
}

void LuaScriptSystem::bindLevel(const Level *level)
{
    lua["level"] = level;
}

sol::state &LuaScriptSystem::getLua()
{
    return lua;
}

void LuaScriptSystem::loadScripts()
{
    sol::protected_function_result result =
        lua.safe_script_file(scriptPath, sol::script_pass_on_error);

    if (!result.valid())
    {
        sol::error scriptError = result;
        throw std::runtime_error(scriptError.what());
    }
}

void LuaScriptSystem::bindPlayer(Player *player)
{
    lua["player"] = player;
}