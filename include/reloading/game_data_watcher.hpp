#pragma once
#include <signals.hpp>
#include "reloading/file_watcher.hpp"

class GameDataWatcher : public FileWatcher
{
public:
    fteng::signal<void()> onGameDataChanged;

    GameDataWatcher()
    {
        listener.onFileModified = [&](const std::string& path) {
            if (path.compare("../../assets/game_data.json") == 0)
            {
                onGameDataChanged();
            }
        };

        fileWatcher.addWatch("../../assets", &listener, false);
        fileWatcher.watch();
    }
};