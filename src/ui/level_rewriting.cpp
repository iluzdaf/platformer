#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "ui/level_rewriting.hpp"
#include "ui/renames.hpp"
#include "game/level_data.hpp"
#include "game/level_data_file.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "tile_map/tile_map_data.hpp"
#include "assets/asset_paths.hpp"

namespace
{
    template <class Spawn> bool renameSpawnTypes(std::vector<Spawn> &spawns, const Renames &renames)
    {
        bool rewritten = false;
        for (Spawn &spawn : spawns)
        {
            auto renamed = renames.find(spawn.type);
            if (renamed == renames.end())
                continue;

            spawn.type = renamed->second;
            rewritten = true;
        }

        return rewritten;
    }

    std::vector<std::string> levelFilesIn(const std::string &directory)
    {
        std::vector<std::string> paths;
        for (const auto &entry : std::filesystem::directory_iterator(assets::pathTo(directory)))
            if (entry.path().extension() == ".json")
                paths.push_back(entry.path().string());

        std::sort(paths.begin(), paths.end());
        return paths;
    }
}

bool rewriting::paletteIn(TileMapData &tileMapData, const Renames &renames)
{
    auto renamed = renames.find(tileMapData.tilePalette);
    if (renamed == renames.end())
        return false;

    tileMapData.tilePalette = renamed->second;
    return true;
}

bool rewriting::typeIn(std::vector<NpcSpawnData> &npcs, const Renames &renames)
{
    return renameSpawnTypes(npcs, renames);
}

bool rewriting::typeIn(std::vector<PickupSpawnData> &pickups, const Renames &renames)
{
    return renameSpawnTypes(pickups, renames);
}

std::vector<std::string> rewriting::theLevels(const std::string &directory, const Rewrite &rewrite)
{
    std::vector<std::string> rewritten;
    for (const std::string &levelPath : levelFilesIn(directory))
    {
        std::optional<LevelData> levelData = readLevelDataIfYouCan(levelPath);
        if (!levelData || !rewrite(*levelData))
            continue;

        writeLevelData(*levelData, levelPath);
        rewritten.push_back(levelPath);
    }

    return rewritten;
}

std::vector<std::string> rewriting::whatItWouldReach(
    const std::string &directory,
    const Rewrite &rewrite)
{
    std::vector<std::string> reached;
    for (const std::string &levelPath : levelFilesIn(directory))
    {
        std::optional<LevelData> levelData = readLevelDataIfYouCan(levelPath);
        if (levelData && rewrite(*levelData))
            reached.push_back(levelPath);
    }

    return reached;
}
