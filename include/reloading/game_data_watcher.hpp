#pragma once
#include <string_view>
#include <signals.hpp>
#include <string>
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
            for (std::string_view named :
                 {assets::GameSettings,
                  assets::Camera,
                  assets::Player,
                  assets::Npcs,
                  assets::TilePalettes})
                if (path == named)
                {
                    onGameDataChanged();
                    return;
                }
        };

        fileWatcher.addWatch(assets::root(), &listener, false);
        fileWatcher.watch();
    }
};