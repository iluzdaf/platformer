#include <algorithm>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include "assets/asset_paths.hpp"

namespace assets
{
    const std::string &root()
    {
        static const std::string resolved =
            std::filesystem::absolute(PLATFORMER_ASSETS_DIR).lexically_normal().string();

        return resolved;
    }

    std::string pathTo(std::string_view relative)
    {
        return (std::filesystem::path(root()) / relative).string();
    }

    std::string underRoot(const std::string &path)
    {
        return std::filesystem::relative(path, root()).generic_string();
    }

    std::vector<std::string> filesIn(std::string_view directory, std::string_view extension)
    {
        std::vector<std::string> paths;
        for (const auto &entry : std::filesystem::directory_iterator(pathTo(directory)))
            if (entry.path().extension() == extension)
                paths.push_back(underRoot(entry.path().string()));

        std::sort(paths.begin(), paths.end());
        return paths;
    }
}
