#pragma once

#include <optional>
#include <string>
#include "game/level_data.hpp"

std::optional<LevelData> readLevelDataIfYouCan(const std::string &levelPath);

LevelData readLevelData(const std::string &levelPath);

void writeLevelData(const LevelData &levelData, const std::string &levelPath);
