#pragma once
#include <efsw/efsw.hpp>
#include <string>
#include <functional>
#include <mutex>
#include <filesystem>
#include "assets/asset_paths.hpp"

class FileWatcherListener : public efsw::FileWatchListener
{
public:
    std::function<void(const std::string &)> onFileModified;

    void handleFileAction(
        efsw::WatchID,
        const std::string &dir,
        const std::string &filename,
        efsw::Action action,
        std::string) override
    {
        if (action == efsw::Actions::Modified || action == efsw::Actions::Add ||
            action == efsw::Actions::Moved)
        {
            std::filesystem::path fullPath = std::filesystem::path(dir) / filename;
            std::lock_guard<std::mutex> lock(queueMutex);
            modifiedFiles.push_back(assets::underRoot(fullPath.string()));
        }
    }

    void process()
    {
        std::vector<std::string> modifiedFilesToProcess;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            modifiedFilesToProcess.swap(modifiedFiles);
        }

        for (const auto &file : modifiedFilesToProcess)
        {
            onFileModified(file);
        }
    }

private:
    std::mutex queueMutex;
    std::vector<std::string> modifiedFiles;
};