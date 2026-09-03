#include <cstddef>
#include <algorithm>
#include <filesystem>
#include <glaze/glaze.hpp>
#include <string>
#include <vector>
#include "game/levels.hpp"
#include "assets/asset_paths.hpp"

std::vector<std::string> levelPathsIn(const std::string &directory)
{
    std::vector<std::string> paths;
    for (const auto &entry : std::filesystem::directory_iterator(assets::pathTo(directory)))
        if (entry.path().extension() == ".json")
            paths.push_back(assets::underRoot(entry.path().string()));

    std::sort(paths.begin(), paths.end());
    return paths;
}

std::string levelName(const std::string &levelPath)
{
    std::string name = levelPath.substr(levelPath.find_last_of("/\\") + 1);
    size_t extension = name.rfind(".json");
    return extension == std::string::npos ? name : name.substr(0, extension);
}

std::string directoryOf(const std::string &levelPath)
{
    return levelPath.substr(0, levelPath.find_last_of("/\\"));
}
