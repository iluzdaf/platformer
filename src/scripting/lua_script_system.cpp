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
#include "conditions/asked.hpp"
#include "game/playback.hpp"
#include "game/level.hpp"
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"
#include "scripting/script_walker.hpp"
#include "actor/behaviors/patrol_data.hpp"
#include "navigation/navigation_place.hpp"
#include "assets/asset_paths.hpp"

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
        "Actor",
        "feet",
        &Actor::feet,
        "standAt",
        &Actor::standAt,
        "alive",
        &Actor::alive,
        "state",
        &Actor::stateName,
        "fact",
        sol::overload(
            sol::resolve<const Asked &(const std::string &) const>(&Actor::fact),
            sol::resolve<void(const std::string &, const Asked &)>(&Actor::fact)),
        "event",
        &Actor::event,
        "threatFeet",
        &Actor::threatFeet,
        "distanceTo",
        &Actor::distanceTo,
        "onSameSurfaceAs",
        &Actor::onSameSurfaceAs,
        "corneredBy",
        &Actor::corneredBy,
        "onGround",
        &Actor::onGround);
    lua.new_usertype<Player>("Player", sol::base_classes, sol::bases<Actor>());
    lua.new_usertype<Npc>(
        "Npc", "type", &Npc::type, "tuning", &Npc::tuning, sol::base_classes, sol::bases<Actor>());
    lua.new_usertype<ScreenTransition>("ScreenTransition", "start", &ScreenTransition::start);
    lua.new_usertype<InputIntentions>(
        "Intentions",
        sol::constructors<InputIntentions()>(),
        "direction",
        &InputIntentions::direction,
        "jumpRequested",
        &InputIntentions::jumpRequested,
        "jumpHeld",
        &InputIntentions::jumpHeld,
        "dashRequested",
        &InputIntentions::dashRequested,
        "climbRequested",
        &InputIntentions::climbRequested,
        "attack",
        &InputIntentions::attack);
    lua.new_usertype<ActorFacts>(
        "ActorFacts",
        sol::no_constructor,
        "feet",
        sol::readonly_property([](const ActorFacts &facts) { return facts.feet; }),
        "threatFeet",
        sol::readonly_property([](const ActorFacts &facts) { return facts.threatFeet; }),
        "onGround",
        sol::readonly_property([](const ActorFacts &facts) { return facts.contacts.onGround; }),
        "beat",
        sol::readonly_property([](const ActorFacts &facts) { return facts.beat; }));
    lua.new_usertype<PatrolData>(
        "Beat",
        sol::no_constructor,
        "from",
        sol::readonly(&PatrolData::from),
        "to",
        sol::readonly(&PatrolData::to));
    lua.new_usertype<PlaceOnThePath>(
        "Place", sol::no_constructor, "feet", sol::readonly(&PlaceOnThePath::feet));
    lua.new_usertype<ScriptWalker>(
        "Walker",
        sol::no_constructor,
        "anchored",
        &ScriptWalker::anchored,
        "finished",
        &ScriptWalker::finished,
        "routeTo",
        sol::overload(
            [](ScriptWalker &walker, int node) { walker.routeTo(node); },
            [](ScriptWalker &walker, int node, glm::vec2 stopShortAt)
            { walker.routeTo(node, stopShortAt); }),
        "follow",
        &ScriptWalker::follow,
        "currentNode",
        &ScriptWalker::currentNode,
        "targetNode",
        &ScriptWalker::targetNode,
        "feetOf",
        &ScriptWalker::feetOf,
        "furthestRefugeFrom",
        &ScriptWalker::furthestRefugeFrom,
        "placeOnThePath",
        &ScriptWalker::placeOnThePath,
        "endOfThePathBeyond",
        &ScriptWalker::endOfThePathBeyond,
        "walkableFrom",
        [](const ScriptWalker &walker, int node)
        { return sol::as_table(walker.walkableFrom(node)); },
        "standsAt",
        &ScriptWalker::standsAt);

    lua.set_function(
        "include",
        [this](const std::string &path) -> sol::object
        {
            sol::environment fresh(lua, sol::create, lua.globals());
            sol::protected_function_result included =
                lua.safe_script_file(assets::pathTo(path), fresh, sol::script_pass_on_error);
            if (!included.valid())
            {
                sol::error error = included;
                throw std::runtime_error(error.what());
            }

            sol::object value = included;
            return value;
        });

    lua.set_function(
        "startCoroutine",
        [this](const sol::function &func)
        {
            sol::thread thread = sol::thread::create(lua.lua_state());
            sol::state_view threadState = thread.state();
            threadState["f"] = func;
            sol::protected_function co = threadState.load("return coroutine.wrap(f)")();
            if (std::optional<float> wait = resume(co, "a coroutine"))
                waitingCoroutines.push_back({thread, co, *wait, startedBy});
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
        report(what, error.what());
        return std::nullopt;
    }

    sol::object yielded = result;
    if (yielded.is<float>())
        return yielded.as<float>();

    return std::nullopt;
}

void LuaScriptSystem::report(std::string_view what, std::string_view why)
{
    std::cerr << "Lua error in " << what << ": " << why << '\n';
    ++reported;
}

int LuaScriptSystem::errorsReported() const
{
    return reported;
}

void LuaScriptSystem::use(std::string_view name, const std::string &path)
{
    NamedScript &script = scripts[std::string(name)];
    script.path = path;
    reload(script, name);
}

void LuaScriptSystem::reload(NamedScript &script, std::string_view name)
{
    sol::environment fresh(lua, sol::create, lua.globals());
    sol::protected_function_result result =
        lua.safe_script_file(script.path, fresh, sol::script_pass_on_error);

    if (!result.valid())
    {
        sol::error scriptError = result;
        report(name, scriptError.what());
        return;
    }

    sol::object handlers = result;
    if (!handlers.is<sol::table>())
    {
        report(name, "a script names no handlers to call");
        return;
    }

    script.environment = fresh;
    script.handlers = handlers.as<sol::table>();
}

sol::object LuaScriptSystem::stateHook(
    std::string_view name,
    const std::string &state,
    std::string_view hook)
{
    auto found = scripts.find(std::string(name));
    if (found == scripts.end() || !found->second.handlers.valid())
        return sol::make_object(lua, sol::lua_nil);

    sol::object states = found->second.handlers["states"];
    if (!states.is<sol::table>())
        return sol::make_object(lua, sol::lua_nil);

    sol::object called = states.as<sol::table>()[state];
    if (!called.is<sol::table>())
        return sol::make_object(lua, sol::lua_nil);

    return called.as<sol::table>()[hook];
}

sol::table LuaScriptSystem::selfOf(const void *owner, const std::string &state)
{
    auto [self, made] = stateSelves.try_emplace({owner, state});
    if (made)
        self->second = lua.create_table();

    return self->second;
}

void LuaScriptSystem::startStateAfresh(const void *owner, const std::string &state)
{
    stateSelves.erase({owner, state});
}

void LuaScriptSystem::forget(const void *owner)
{
    std::erase_if(
        waitingCoroutines,
        [owner](const WaitingCoroutine &waiting) { return waiting.startedBy == owner; });
    std::erase_if(stateSelves, [owner](const auto &self) { return self.first.first == owner; });
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

    for (auto &[name, script] : scripts)
        reload(script, name);
}

void LuaScriptSystem::bindPlayer(Player *player)
{
    lua["player"] = player;
}