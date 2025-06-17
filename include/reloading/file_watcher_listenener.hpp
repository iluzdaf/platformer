#pragma once
#include <efsw/efsw.hpp>
#include <string>
#include <functional>
#include <mutex>
#include <filesystem>

class FileWatcherListener : public efsw::FileWatchListener
{
public:
    std::function<void(const std::string &)> onFileModified;

    void handleFileAction(
        efsw::WatchID,
        const std::string &dir,
        const std::string &filename,
        efsw::Action action,
        std::string oldFilename = "") override
    {
        if (action == efsw::Actions::Modified ||
            action == efsw::Actions::Add ||
            action == efsw::Actions::Moved)
        {
            std::filesystem::path fullPath = std::filesystem::path(dir) / filename;
            std::filesystem::path relativePath = std::filesystem::relative(fullPath, projectRoot);
            std::filesystem::path finalPath = std::filesystem::path("..") / relativePath;
            std::lock_guard<std::mutex> lock(queueMutex);
            modifiedFiles.push_back(finalPath.string());
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
    std::filesystem::path projectRoot = std::filesystem::absolute("../");
};