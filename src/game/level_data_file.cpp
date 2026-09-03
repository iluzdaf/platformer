#include <algorithm>
#include <cstddef>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <glaze/glaze.hpp>
#include "game/level_data_file.hpp"
#include "game/level_data.hpp"
#include "serialization/json_format.hpp"
#include "assets/asset_paths.hpp"

namespace
{
    std::string withPaddedGrid(const std::string &json)
    {
        const std::string key = "\"indices\":[";
        size_t start = json.find(key);
        if (start == std::string::npos)
            return json;

        size_t cursor = start + key.size();
        std::vector<std::vector<std::string>> rows;

        while (cursor < json.size() && json[cursor] == '[')
        {
            size_t end = json.find(']', cursor);
            if (end == std::string::npos)
                return json;

            std::vector<std::string> cells;
            for (size_t cell = cursor + 1; cell < end;)
            {
                size_t comma = json.find(',', cell);
                if (comma == std::string::npos || comma > end)
                    comma = end;

                cells.push_back(json.substr(cell, comma - cell));
                cell = comma + 1;
            }
            rows.push_back(std::move(cells));

            cursor = end + 1;
            if (cursor < json.size() && json[cursor] == ',')
                ++cursor;
        }

        size_t width = 0;
        for (const auto &row : rows)
            for (const auto &cell : row)
                width = std::max(width, cell.size());

        std::string out = json.substr(0, start + key.size());
        for (size_t row = 0; row < rows.size(); ++row)
        {
            out += "[";
            for (size_t cell = 0; cell < rows[row].size(); ++cell)
            {
                if (cell > 0)
                    out += ",";

                out += std::string(width - rows[row][cell].size(), ' ') + rows[row][cell];
            }
            out += "]";

            if (row + 1 < rows.size())
                out += ",";
        }

        return out + json.substr(cursor);
    }
}

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
    std::string json;
    if (glz::write_json(levelData, json))
        throw std::runtime_error("Failed to serialize LevelData to JSON");

    std::ofstream outFile(assets::pathTo(levelPath));
    outFile << withStructureOnLines(withPaddedGrid(json));
}
