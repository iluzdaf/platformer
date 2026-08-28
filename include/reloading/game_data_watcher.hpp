#pragma once
#include <signals.hpp>
#include "reloading/file_watcher.hpp"
#include "assets/asset_paths.hpp"

class GameDataWatcher : public FileWatcher
{
public:
    fteng::signal<void()> onGameDataChanged;

    GameDataWatcher()
    {
        listener.onFileModified = [&](const std::string &path)
        {
            if (path == assets::GameData)
                onGameDataChanged();
        };

        fileWatcher.addWatch(assets::root(), &listener, false);
        fileWatcher.watch();
    }
};