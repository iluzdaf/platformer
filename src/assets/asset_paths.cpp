#include <filesystem>
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
}
