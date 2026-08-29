#pragma once

#include "game/game.hpp"
#include "reloading/hot_reload.hpp"
#include "window/window.hpp"

class App
{
public:
    App();
    void run();

private:
    Window window;
    HotReload hotReload;
    Game game;
};
