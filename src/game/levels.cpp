#include <algorithm>
#include <cstddef>
#include <glaze/glaze.hpp>
#include <string>
#include <vector>
#include "game/levels.hpp"
#include "assets/asset_paths.hpp"

std::vector<std::string> levelPathsIn(const std::string &directory)
{
    return assets::filesIn(directory, ".json");
}

std::string aLevelPathNobodyHasTaken(
    const std::string &directory,
    const std::vector<std::string> &alsoTaken)
{
    std::vector<std::string> taken = levelPathsIn(directory);
    taken.insert(taken.end(), alsoTaken.begin(), alsoTaken.end());
    for (std::size_t suffix = taken.size() + 1;; ++suffix)
    {
        std::string path = directory + "/level" + std::to_string(suffix) + ".json";
        if (std::find(taken.begin(), taken.end(), path) == taken.end())
            return path;
    }
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
