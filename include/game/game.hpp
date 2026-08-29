#pragma once

#include <memory>
#include <vector>
#include <signals.hpp>
#include "rendering/ui/game_ui.hpp"
#include "game/game_data.hpp"
#include "game/levels.hpp"
#include "tile_map/tile_interaction_system.hpp"
#include "actor/actor.hpp"
#include "player/player.hpp"
#include "npc/npc.hpp"
#include "game/scoring_system.hpp"
#include "rendering/game_renderer.hpp"
#include "rendering/screen_transition.hpp"
#include "cameras/camera2d.hpp"
#include "input/keyboard_manager.hpp"
#include "input/input_manager.hpp"
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
    void loadLevel(const std::string &levelPath);
    void rebuildPlayer();

private:
    void reload();
    void reloadScripts();
    bool isPlaying(const std::string &levelPath) const;
    void rebuildNpcs();
    void refreshActors();
    void preFixedUpdate();
    void fixedUpdate(float deltaTime);
    void postFixedUpdate();
    void update(float deltaTime);
    void render();
    void resize(int width, int height);
    void rebuildLevel(const std::string &levelPath);

    Window &window;
    GameData gameData;
    Camera2D camera;
    KeyboardManager keyboardManager;
    InputManager inputManager;
    Playback playback;
    LuaScriptSystem luaScriptSystem;
    std::unique_ptr<Level> level;
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Npc>> npcs;
    std::vector<Actor *> actors;
    TileInteractionSystem tileInteractionSystem;
    ScoringSystem scoringSystem;
    GameRenderer renderer;
    ScreenTransition screenTransition;
    GameUi gameUi;
    Levels levels;
    fteng::connection onLevelCompleteConnection, onResizeConnection;
    std::vector<fteng::connection> reloadConnections;
    bool showEditors = false;
};