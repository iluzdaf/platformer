#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <iterator>
#include "npc/npc_spawn_data.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/asset_path.hpp"
#include "tile_map/tile_map_data.hpp"

namespace
{
    Level loadLevel(const std::string &path)
    {
        return Level(path, shippedPalettes(), PlayerData(), shippedNpcData());
    }

    void layFloor(TileMap &tileMap, int groundY, int fromX, int toX)
    {
        for (int x = fromX; x <= toX; ++x)
            tileMap.setTileIndex(glm::ivec2(x, groundY), 1);
    }

    std::string readFile(const std::filesystem::path &path)
    {
        std::ifstream file(path);
        return std::string(
            (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

}

TEST_CASE("A saved level carries no navigation data", "[TileMap][Navigation]")
{
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 0, 9);

    std::string json;
    REQUIRE_FALSE(glz::write_json(tileMap.toTileMapData(), json));
    REQUIRE(json.find("navigation") == std::string::npos);
}

TEST_CASE(
    "Saving a level writes a readable grid and tile table, and reloads unchanged",
    "[TileMap][Level]")
{
    std::filesystem::path savePath =
        std::filesystem::temp_directory_path() / "platformer_save_roundtrip.json";
    std::filesystem::copy_file(
        assetPath("levels/level6.json"),
        savePath,
        std::filesystem::copy_options::overwrite_existing);

    Level loaded = loadLevel(savePath.string());
    loaded.save();

    std::string savedJson = readFile(savePath);

    REQUIRE(savedJson.starts_with("{\n    \""));
    REQUIRE(savedJson.ends_with("\n}"));

    REQUIRE(savedJson.find("\"indices\":[\n            [") != std::string::npos);

    size_t rows = 0;
    for (size_t at = savedJson.find("\n            ["); at != std::string::npos;
         at = savedJson.find("\n            [", at + 1))
        ++rows;
    REQUIRE(rows == static_cast<size_t>(loaded.getTileMap().getHeight()));

    REQUIRE(savedJson.find("\"tilePalette\":\"default\"") != std::string::npos);
    REQUIRE(savedJson.find("\"tileData\"") == std::string::npos);

    REQUIRE(savedJson.find("\"playerStartTilePosition\":[") != std::string::npos);
    REQUIRE(savedJson.find("\"playerStartTilePosition\":[\n") == std::string::npos);

    Level reloaded = loadLevel(savePath.string());
    reloaded.save();
    REQUIRE(readFile(savePath) == savedJson);

    REQUIRE(reloaded.getTileMap().getWidth() == loaded.getTileMap().getWidth());
    REQUIRE(reloaded.getTileMap().getHeight() == loaded.getTileMap().getHeight());
    REQUIRE(reloaded.getTileMap().getTileSize() == loaded.getTileMap().getTileSize());
    REQUIRE(reloaded.getNextLevel() == loaded.getNextLevel());
    REQUIRE(reloaded.getNpcs() == loaded.getNpcs());
    REQUIRE(reloaded.getPlayerStartTile() == loaded.getPlayerStartTile());

    for (int y = 0; y < loaded.getTileMap().getHeight(); ++y)
        for (int x = 0; x < loaded.getTileMap().getWidth(); ++x)
            REQUIRE(
                reloaded.getTileMap().tilePositionToTileIndex({x, y}) ==
                loaded.getTileMap().tilePositionToTileIndex({x, y}));

    std::filesystem::remove(savePath);
}

TEST_CASE("Every shipped level is already in the format the editor saves", "[TileMap][Level]")
{
    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
    {
        if (entry.path().extension() != ".json")
            continue;

        std::string original = readFile(entry.path());

        std::filesystem::path savePath =
            std::filesystem::temp_directory_path() / entry.path().filename();
        std::filesystem::copy_file(
            entry.path(), savePath, std::filesystem::copy_options::overwrite_existing);

        loadLevel(savePath.string()).save();

        INFO(
            "level " << entry.path().filename().string() << " is not saved in the editor's format");
        REQUIRE(readFile(savePath) == original);

        std::filesystem::remove(savePath);
    }
}

TEST_CASE("A level knows where it heads next", "[TileMap][Level]")
{
    Level level = loadLevel(assetPath("levels/level6.json"));

    REQUIRE(level.getNextLevel() == "levels/level1.json");
    REQUIRE_FALSE(level.getNpcs().empty());
    for (const NpcSpawnData &spawn : level.getNpcs())
        REQUIRE(shippedNpcData().contains(spawn.type));

    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
        if (entry.path().extension() == ".json")
            REQUIRE_FALSE(loadLevel(entry.path().string()).getNextLevel().empty());
}

TEST_CASE("A level naming a palette that does not exist fails to load", "[TileMap][Level]")
{
    TileMapData tileMapData;
    tileMapData.width = 4;
    tileMapData.height = 4;
    tileMapData.tilePalette = "nope";

    REQUIRE_THROWS_WITH(TileMap(tileMapData, shippedPalettes()), "Unknown tile palette \"nope\"");
}

TEST_CASE("Every shipped level uses a palette that exists", "[TileMap][Level]")
{
    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
        if (entry.path().extension() == ".json")
            REQUIRE_NOTHROW(loadLevel(entry.path().string()));
}
