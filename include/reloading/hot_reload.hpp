#pragma once

#include <efsw/efsw.hpp>
#include "reloading/file_watcher_listener.hpp"
#include "reloading/reloader.hpp"

class HotReload
{
public:
    HotReload();
    void process();

    Reloader &getReloader();

private:
    Reloader reloader;
    FileWatcherListener listener;
    efsw::FileWatcher fileWatcher;
};
