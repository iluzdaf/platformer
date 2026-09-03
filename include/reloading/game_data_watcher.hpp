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

    static bool reloadsOn(std::string_view path)
    {
        for (std::string_view named :
             {assets::GameSettings,
              assets::Camera,
              assets::Player,
              assets::Npcs,
              assets::Pickups,
              assets::TilePalettes,
              assets::LevelList})
            if (path == named)
                return true;

        return false;
    }

    GameDataWatcher()
    {
        listener.onFileModified = [&](const std::string &path)
        {
            if (reloadsOn(path))
                onGameDataChanged();
        };

        fileWatcher.addWatch(assets::root(), &listener, false);
        fileWatcher.watch();
    }
};