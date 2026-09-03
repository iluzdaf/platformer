#include <map>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <glaze/glaze.hpp>
#include "game/game_data.hpp"
#include "game/levels_data.hpp"
#include "pickups/pickup_data.hpp"
#include "cameras/camera2d_data.hpp"
#include "player/player_data.hpp"
#include "npc/npc_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "assets/asset_paths.hpp"
#include "serialization/json_format.hpp"

namespace
{
    template <typename T> void readInto(T &into, std::string_view asset)
    {
        std::string path = assets::pathTo(asset);
        auto error = glz::read_file_json(into, path, std::string{});
        if (error)
            throw std::runtime_error("Failed to read " + path);
    }

    template <typename T> void writeTo(const T &value, std::string_view asset)
    {
        std::string path = assets::pathTo(asset);
        std::string json;
        if (glz::write_json(value, json))
            throw std::runtime_error("Failed to serialise " + path);

        std::ofstream out(path);
        out << withStructureOnLines(json);
    }
}

void saveGameSettings(const GameSettingsData &settings)
{
    writeTo(settings, assets::GameSettings);
}

void saveCameraData(const Camera2DData &cameraData)
{
    writeTo(cameraData, assets::Camera);
}

void savePlayerData(const PlayerData &playerData)
{
    writeTo(playerData, assets::Player);
}

void saveNpcData(const std::map<std::string, NpcData> &npcData)
{
    writeTo(npcData, assets::Npcs);
}

void savePickupData(const std::map<std::string, PickupData> &pickupData)
{
    writeTo(pickupData, assets::Pickups);
}

void saveLevels(const LevelsData &levels)
{
    writeTo(levels, assets::LevelList);
}

void saveTilePalettes(const TilePalettes &tilePalettes)
{
    writeTo(tilePalettes, assets::TilePalettes);
}

GameData loadGameData()
{
    GameData loaded;
    readInto(loaded.settings, assets::GameSettings);
    readInto(loaded.cameraData, assets::Camera);
    readInto(loaded.playerData, assets::Player);
    readInto(loaded.npcData, assets::Npcs);
    readInto(loaded.pickupData, assets::Pickups);
    readInto(loaded.tilePalettes, assets::TilePalettes);
    readInto(loaded.levels, assets::LevelList);

    if (loaded.levels.first.empty())
        throw std::runtime_error("first must not be empty");

    return loaded;
}
