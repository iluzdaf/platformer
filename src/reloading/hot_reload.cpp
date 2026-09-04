#include <string>
#include "reloading/hot_reload.hpp"
#include "assets/asset_paths.hpp"

HotReload::HotReload()
{
    listener.onFileModified = [this](const std::string &path) { reloader.fileChanged(path); };

    fileWatcher.addWatch(assets::root(), &listener, true);
    fileWatcher.watch();
}

void HotReload::process()
{
    listener.process();
}

Reloader &HotReload::getReloader()
{
    return reloader;
}
