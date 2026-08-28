#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <glaze/glaze.hpp>
#include <string>
#include <vector>
#include "game/levels.hpp"
#include "assets/asset_paths.hpp"
#include "game/levels_data.hpp"

Levels::Levels(const std::string &jsonFilePath) : path(jsonFilePath)
{
    LevelsData levelsData;
    auto error = glz::read_file_json(levelsData, path, std::string{});
    if (error)
        throw std::runtime_error("Failed to read levels json file " + path);

    first = levelsData.first;
    if (first.empty())
        throw std::runtime_error("first must not be empty");
}

const std::string &Levels::getFirst() const
{
    return first;
}

void Levels::setFirst(const std::string &levelPath)
{
    if (levelPath.empty())
        throw std::runtime_error("first must not be empty");

    first = levelPath;
}

void Levels::save() const
{
    std::string json;
    auto error = glz::write_json(LevelsData{first}, json);
    if (error)
        throw std::runtime_error("Failed to serialize LevelsData to JSON");

    std::ofstream outFile(path);
    outFile << json;
}

std::vector<std::string> levelPathsIn(const std::string &directory)
{
    std::vector<std::string> paths;
    for (const auto &entry : std::filesystem::directory_iterator(assets::pathTo(directory)))
        if (entry.path().extension() == ".json")
            paths.push_back(assets::underRoot(entry.path().string()));

    std::sort(paths.begin(), paths.end());
    return paths;
}
