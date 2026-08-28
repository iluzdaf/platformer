#include <stdexcept>
#include <string>
#include <string_view>
#include <glaze/glaze.hpp>
#include "game/game_data.hpp"
#include "assets/asset_paths.hpp"

namespace
{
    template <typename T> void readInto(T &into, std::string_view asset)
    {
        std::string path = assets::pathTo(asset);
        auto error = glz::read_file_json(into, path, std::string{});
        if (error)
            throw std::runtime_error("Failed to read " + path);
    }
}

GameData loadGameData()
{
    GameData loaded;
    readInto(loaded.settings, assets::GameSettings);
    readInto(loaded.cameraData, assets::Camera);
    readInto(loaded.playerData, assets::Player);
    readInto(loaded.npcData, assets::Npcs);
    readInto(loaded.tilePalettes, assets::TilePalettes);

    return loaded;
}
