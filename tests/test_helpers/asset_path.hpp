#pragma once

#include <string>

inline std::string assetPath(const std::string &relativePath)
{
    return std::string(PLATFORMER_ASSETS_DIR) + "/" + relativePath;
}
