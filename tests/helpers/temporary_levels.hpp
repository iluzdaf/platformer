#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include "game/level_data.hpp"
#include "game/level_data_file.hpp"
#include "helpers/asset_path.hpp"

struct TemporaryLevels
{
    std::filesystem::path directory;

    explicit TemporaryLevels(const std::string &name)
        : directory(std::filesystem::temp_directory_path() / ("platformer_" + name))
    {
        std::filesystem::remove_all(directory);
        std::filesystem::create_directories(directory);
    }

    ~TemporaryLevels()
    {
        std::filesystem::remove_all(directory);
    }

    TemporaryLevels(const TemporaryLevels &) = delete;
    TemporaryLevels &operator=(const TemporaryLevels &) = delete;
    TemporaryLevels(TemporaryLevels &&) = delete;
    TemporaryLevels &operator=(TemporaryLevels &&) = delete;

    std::string pathOf(const std::string &file) const
    {
        return (directory / file).string();
    }

    void write(const std::string &file, const LevelData &levelData) const
    {
        writeLevelData(levelData, pathOf(file));
    }

    void copyShipped(const std::string &file) const
    {
        std::filesystem::copy_file(assetPath("levels/" + file), directory / file);
    }

    void writeText(const std::string &file, const std::string &text) const
    {
        std::ofstream(directory / file) << text;
    }
};
