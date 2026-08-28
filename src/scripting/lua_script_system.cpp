#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
#include "scripting/lua_script_system.hpp"
#include "game/game.hpp"
#include "game/level.hpp"

LuaScriptSystem::LuaScriptSystem(const std::string &scriptPath)
    : scriptPath(scriptPath)
{
    lua.open_libraries(
        sol::lib::base,
        sol::lib::math,
        sol::lib::table,
        sol::lib::string,
        sol::lib::coroutine);

    lua.new_usertype<glm::vec2>("vec2",
                                sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
                                "x", &glm::vec2::x,
                                "y", &glm::vec2::y);
    lua.new_usertype<Game>("Game", "pause", &Game::pause,
                           "play", &Game::play,
                           "loadLevel", &Game::loadLevel,
                           "rebuildPlayer", &Game::rebuildPlayer);
    lua.new_usertype<Camera2D>("Camera", "startShake", &Camera2D::startShake);
    lua.new_usertype<Level>("Level", "getNextLevel", &Level::getNextLevel);
    lua.new_usertype<Player>("Player", "setPosition", &Player::setPosition);
    lua.new_usertype<ScreenTransition>("ScreenTransition", "start", &ScreenTransition::start);

    lua.set_function("startCoroutine", [this](sol::function func)
                     {
        sol::thread thread = sol::thread::create(lua.lua_state());
        sol::state_view threadState = thread.state();
        threadState["f"] = func;
        sol::function co = threadState.load("return coroutine.wrap(f)")();
        sol::object result = co();
        if (result.valid() && result.is<float>())
        {
            float wait = result.as<float>();
            waitingCoroutines.push_back({thread, co, wait});
        } });

    loadScripts();
}

void LuaScriptSystem::update(float deltaTime)
{
    for (auto it = waitingCoroutines.begin(); it != waitingCoroutines.end();)
    {
        it->remainingTime -= deltaTime;
        if (it->remainingTime <= 0.0f)
        {
            sol::object result = it->co();

            if (result.valid() && result.is<float>())
            {
                it->remainingTime = result.as<float>();
                ++it;
            }
            else
            {
                it = waitingCoroutines.erase(it);
            }
        }
        else
        {
            ++it;
        }
    }
}

void LuaScriptSystem::bindGameObjects(
    Game *game,
    Camera2D *camera,
    ScreenTransition *screenTransition)
{
    lua["game"] = game;
    lua["camera"] = camera;
    lua["screenTransition"] = screenTransition;
}

void LuaScriptSystem::triggerLevelComplete()
{
    if (onLevelComplete.valid())
        onLevelComplete();
}

void LuaScriptSystem::triggerDeath()
{
    if (onDeath.valid())
        onDeath();
}

void LuaScriptSystem::bindLevel(Level *level)
{
    lua["level"] = level;
}

sol::state &LuaScriptSystem::getLua()
{
    return lua;
}

const std::vector<LuaScriptSystem::WaitingCoroutine> &LuaScriptSystem::getWaitingCoroutines() const
{
    return waitingCoroutines;
}

void LuaScriptSystem::triggerWallJump()
{
    if (onWallJump.valid())
        onWallJump();
}

void LuaScriptSystem::triggerDoubleJump()
{
    if (onDoubleJump.valid())
        onDoubleJump();
}

void LuaScriptSystem::triggerDash()
{
    if (onDash.valid())
        onDash();
}

void LuaScriptSystem::triggerFallFromHeight()
{
    if (onFallFromHeight.valid())
        onFallFromHeight();
}

void LuaScriptSystem::triggerHitCeiling()
{
    if (onHitCeiling.valid())
        onHitCeiling();
}

void LuaScriptSystem::triggerWallSliding()
{
    if (onWallSliding.valid())
        onWallSliding();
}

void LuaScriptSystem::loadScripts()
{
    sol::protected_function_result result = lua.safe_script_file(
        scriptPath,
        sol::script_pass_on_error);

    if (!result.valid())
    {
        sol::error scriptError = result;
        throw std::runtime_error(scriptError.what());
    }

    onDeath = lua["onDeath"];
    onLevelComplete = lua["onLevelComplete"];
    onWallJump = lua["onWallJump"];
    onDoubleJump = lua["onDoubleJump"];
    onDash = lua["onDash"];
    onFallFromHeight = lua["onFallFromHeight"];
    onHitCeiling = lua["onHitCeiling"];
    onWallSliding = lua["onWallSliding"];
    onGameLoaded = lua["onGameLoaded"];
}

void LuaScriptSystem::triggerGameLoaded()
{
    if (onGameLoaded.valid())
        onGameLoaded();
}

void LuaScriptSystem::bindPlayer(Player *player)
{
    lua["player"] = player;
}