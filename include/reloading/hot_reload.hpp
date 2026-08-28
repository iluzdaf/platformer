#pragma once

#include "reloading/asset_watcher.hpp"
#include "reloading/game_data_watcher.hpp"
#include "reloading/level_watcher.hpp"
#include "reloading/reload_commands.hpp"
#include "reloading/reloader.hpp"
#include "reloading/script_watcher.hpp"

class HotReload
{
public:
    HotReload();
    void process();

    ReloadCommands &commands();

private:
    Reloader reloader;
    LevelWatcher levelWatcher;
    AssetWatcher assetWatcher;
    GameDataWatcher gameDataWatcher;
    ScriptWatcher scriptWatcher;
};
