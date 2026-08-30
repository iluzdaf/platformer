#pragma once

#include <string>
#include <vector>
#include <signals.hpp>
#include "ui/game_ui.hpp"
#include "game/game_data.hpp"
#include "game/levels.hpp"
#include "game/world.hpp"
#include "rendering/game_renderer.hpp"
#include "rendering/screen_transition.hpp"
#include "cameras/camera2d.hpp"
#include "input/keyboard_manager.hpp"
#include "input/keyboard_intentions.hpp"
#include "game/playback.hpp"
#include "scripting/lua_script_system.hpp"
#include "window/window.hpp"

struct ReloadCommands;

class Game
{
public:
    Game(Window &window, ReloadCommands &reloadCommands);
    ~Game();
    void frame(float deltaTime);

private:
    void render();

    Window &window;
    GameData gameData;
    Camera2D camera;
    KeyboardManager keyboardManager;
    KeyboardIntentions keyboardIntentions;
    Playback playback;
    LuaScriptSystem luaScriptSystem;
    World world;
    GameRenderer renderer;
    ScreenTransition screenTransition;
    GameUi gameUi;
    Levels levels;
    fteng::connection onResizeConnection;
    std::vector<fteng::connection> reloadConnections;
    bool showEditors = false;
};
