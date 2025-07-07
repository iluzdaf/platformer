#pragma once
#include <signals.hpp>
#include "reloading/file_watcher.hpp"

class AssetWatcher : public FileWatcher
{
public:
    fteng::signal<void(const std::string &)> onTextureChanged;
    fteng::signal<void(const std::string &)> onShaderChanged;

    AssetWatcher()
    {
        listener.onFileModified = [&](const std::string &path)
        {
            if (endsWith(path, ".png"))
                onTextureChanged(path);
            else if (endsWith(path, ".fs") || endsWith(path, ".vs"))
                onShaderChanged(path);
        };

        fileWatcher.addWatch("../../assets/textures", &listener, false);
        fileWatcher.addWatch("../../assets/shaders", &listener, false);
        fileWatcher.watch();
    }
};