#pragma once
#include <efsw/efsw.hpp>
#include "reloading/file_watcher_listenener.hpp"

class FileWatcher
{
public:
    void process()
    {
        listener.process();
    }

protected:
    efsw::FileWatcher fileWatcher;
    FileWatcherListener listener;

    bool endsWith(const std::string &s, const std::string &suffix) const
    {
        return s.size() >= suffix.size() &&
               s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
};