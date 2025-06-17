#pragma once
#include <signals.hpp>
#include "reloading/file_watcher.hpp"

class LevelWatcher : public FileWatcher
{
public:
    fteng::signal<void(const std::string &)> onLevelChanged;

    LevelWatcher()
    {
        listener.onFileModified = [&](const std::string &path)
        {
            if (endsWith(path, ".json"))
                onLevelChanged(path);
        };

        fileWatcher.addWatch("../assets/levels", &listener, false);
        fileWatcher.watch();
    }
};