#include <glm/gtc/matrix_transform.hpp>
#include "scripting/lua_script_system.hpp"
#include "game/game.hpp"

LuaScriptSystem::LuaScriptSystem()
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
                           "loadNextLevel", &Game::loadNextLevel);
    lua.new_usertype<Camera2D>("Camera", "startShake", &Camera2D::startShake);
    lua.new_usertype<TileMap>("TileMap", "getPlayerStartWorldPosition", &TileMap::getPlayerStartWorldPosition);
    lua.new_usertype<Player>("Player", "setPosition", &Player::setPosition,
                             "reset", &Player::reset);
    lua.new_usertype<ScreenTransition>("ScreenTransition", "start", &ScreenTransition::start);

    lua.set_function("startCoroutine", [this](sol::function func)
                     {
        sol::thread thread = sol::thread::create(lua.lua_state());
        sol::state_view thread_state = thread.state();
        thread_state["f"] = func;
        sol::function co = thread_state.load("return coroutine.wrap(f)")();
        sol::object result = co();
        if (result.valid() && result.is<float>())
        {
            float wait = result.as<float>();
            waitingCoroutines.push_back({thread, co, wait});
        } });

    lua.script_file("../assets/scripts/game_logic.lua");
    onDeath = lua["onDeath"];
    onLevelComplete = lua["onLevelComplete"];
    onWallJump = lua["onWallJump"];
    onDoubleJump = lua["onDoubleJump"];
    onDash = lua["onDash"];
    onFallFromHeight = lua["onFallFromHeight"];
    onHitCeiling = lua["onHitCeiling"];
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
    TileMap *tileMap,
    Player *player,
    ScreenTransition *screenTransition)
{
    lua["game"] = game;
    lua["camera"] = camera;
    lua["tileMap"] = tileMap;
    lua["player"] = player;
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

void LuaScriptSystem::rebindTileMap(TileMap *tileMap)
{
    lua["tileMap"] = tileMap;
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