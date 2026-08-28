#include <stdexcept>
#include <string>
#include <glaze/glaze.hpp>
#include "game/game_data.hpp"
#include "assets/asset_paths.hpp"

GameData loadGameData()
{
    GameData loaded;
    auto error = glz::read_file_json(loaded, assets::pathTo(assets::GameData), std::string{});
    if (error)
        throw std::runtime_error("Failed to read game data json file");

    return loaded;
}
