#pragma once
#include <signals.hpp>
#include <string>
#include "reloading/file_watcher.hpp"
#include "assets/asset_paths.hpp"

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

        fileWatcher.addWatch(assets::pathTo(assets::Textures), &listener, false);
        fileWatcher.addWatch(assets::pathTo(assets::Shaders), &listener, false);
        fileWatcher.watch();
    }
};