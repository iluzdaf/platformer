#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include "assets/asset_paths.hpp"
#include "game/game_data.hpp"
#include "serialization/json_format.hpp"

namespace
{
    std::string committed(std::string_view asset)
    {
        std::ifstream file(assets::pathTo(asset));
        return std::string(
            (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
}

TEST_CASE("Saving a shipped data file as it was loaded changes nothing for git", "[GameData]")
{
    GameData gameData = loadGameData();

    REQUIRE(asFileText(gameData.settings) == committed(assets::GameSettings));
    REQUIRE(asFileText(gameData.cameraData) == committed(assets::Camera));
    REQUIRE(asFileText(gameData.playerData) == committed(assets::Player));
    REQUIRE(asFileText(gameData.npcData) == committed(assets::Npcs));
    REQUIRE(asFileText(gameData.pickupData) == committed(assets::Pickups));
    REQUIRE(asFileText(gameData.tilePalettes) == committed(assets::TilePalettes));
    REQUIRE(asFileText(gameData.levels) == committed(assets::LevelList));
}
