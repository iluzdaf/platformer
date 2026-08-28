#pragma once

#include "reloading/asset_watcher.hpp"
#include "reloading/game_data_watcher.hpp"
#include "reloading/level_watcher.hpp"
#include "reloading/script_watcher.hpp"

class Game;

class HotReload
{
public:
    explicit HotReload(Game &game);
    void process();

private:
    Game &game;
    LevelWatcher levelWatcher;
    AssetWatcher assetWatcher;
    GameDataWatcher gameDataWatcher;
    ScriptWatcher scriptWatcher;
};
