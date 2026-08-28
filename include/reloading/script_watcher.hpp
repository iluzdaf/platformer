#pragma once
#include <signals.hpp>
#include "reloading/file_watcher.hpp"
#include "assets/asset_paths.hpp"

class ScriptWatcher : public FileWatcher
{
public:
    fteng::signal<void()> onScriptsChanged;

    ScriptWatcher()
    {
        listener.onFileModified = [&](const std::string &) { onScriptsChanged(); };

        fileWatcher.addWatch(assets::pathTo(assets::Scripts), &listener, false);
        fileWatcher.watch();
    }
};