#pragma once

#include "reloading/asset_watcher.hpp"
#include "reloading/game_data_watcher.hpp"
#include "reloading/level_watcher.hpp"
#include "reloading/script_watcher.hpp"

class Game;

// Watches the assets a running game was built from and tells it what changed.
// The game knows how to rebuild itself; it does not need to know that anyone is
// watching, which is why this sits beside it rather than inside it.
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
