#pragma once

#include <string>
#include "assets/asset_paths.hpp"

inline std::string assetPath(const std::string &relativePath)
{
    return assets::pathTo(relativePath);
}
