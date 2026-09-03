#pragma once

#include <string>
#include <vector>

std::vector<std::string> levelPathsIn(const std::string &directory);
std::string levelName(const std::string &levelPath);
std::string directoryOf(const std::string &levelPath);
