#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <glaze/glaze.hpp>
#include "game/level_data_file.hpp"
#include "game/level_data.hpp"
#include "serialization/json_format.hpp"
#include "assets/asset_paths.hpp"

std::optional<LevelData> readLevelDataIfYouCan(const std::string &levelPath)
{
    LevelData levelData;
    if (glz::read_file_json(levelData, assets::pathTo(levelPath), std::string{}))
        return std::nullopt;

    return levelData;
}

LevelData readLevelData(const std::string &levelPath)
{
    std::optional<LevelData> read = readLevelDataIfYouCan(levelPath);
    if (!read)
        throw std::runtime_error("Failed to read level json file " + levelPath);

    return std::move(*read);
}

void writeLevelData(const LevelData &levelData, const std::string &levelPath)
{
    std::ofstream outFile(assets::pathTo(levelPath));
    outFile << asFileText(levelData);
}
