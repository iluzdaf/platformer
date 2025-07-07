#pragma once
#include <signals.hpp>
#include "reloading/file_watcher.hpp"

class ScriptWatcher : public FileWatcher
{
public:
    fteng::signal<void()> onScriptsChanged;

    ScriptWatcher()
    {
        listener.onFileModified = [&](const std::string &)
        {
            onScriptsChanged();
        };

        fileWatcher.addWatch("../../assets/scripts", &listener, false);
        fileWatcher.watch();
    }
};