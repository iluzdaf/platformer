#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include "game/tile_map/tile_map.hpp"
#include "test_helpers/test_tile_map_utils.hpp"

namespace
{
    void layFloor(TileMap &tileMap, int groundY, int fromX, int toX)
    {
        for (int x = fromX; x <= toX; ++x)
            tileMap.setTileIndex(glm::ivec2(x, groundY), 1);
    }

    std::vector<float> nodeXsOnRow(const TileMap &tileMap, float y)
    {
        std::vector<float> xs;
        for (const auto &[id, node] : tileMap.getNavigationGraph().getNodes())
            if (node.position.y == y)
                xs.push_back(node.position.x);
        std::sort(xs.begin(), xs.end());
        return xs;
    }

    int nodeIdAt(const TileMap &tileMap, float x, float y)
    {
        for (const auto &[id, node] : tileMap.getNavigationGraph().getNodes())
            if (node.position == glm::vec2(x, y))
                return id;
        return -1;
    }

    bool isReachable(const TileMap &tileMap, glm::vec2 from, glm::vec2 to)
    {
        const NavigationGraph &navigationGraph = tileMap.getNavigationGraph();
        int fromId = nodeIdAt(tileMap, from.x, from.y);
        int toId = nodeIdAt(tileMap, to.x, to.y);
        if (fromId < 0 || toId < 0)
            return false;

        std::set<int> seen{fromId};
        std::vector<int> pending{fromId};
        while (!pending.empty())
        {
            int nodeId = pending.back();
            pending.pop_back();
            if (nodeId == toId)
                return true;

            for (const auto &edge : navigationGraph.getOutgoingEdges(nodeId))
                if (seen.insert(edge.toId).second)
                    pending.push_back(edge.toId);
        }
        return false;
    }

    std::string readFile(const std::filesystem::path &path)
    {
        std::ifstream file(path);
        return std::string(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
    }

    bool hasEdgeBetween(const TileMap &tileMap, float fromX, float toX, float y)
    {
        const NavigationGraph &navigationGraph = tileMap.getNavigationGraph();
        for (const auto &edge : navigationGraph.getEdges())
        {
            NavigationNode from = navigationGraph.getNode(edge.fromId);
            NavigationNode to = navigationGraph.getNode(edge.toId);
            if (from.position == glm::vec2(fromX, y) && to.position == glm::vec2(toX, y))
                return true;
        }
        return false;
    }
}

TEST_CASE("Walk edges are bidirectional along a floor", "[TileMap][Navigation]")
{
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 2, 4);
    tileMap.buildNavigationGraph();

    const NavigationGraph &navigationGraph = tileMap.getNavigationGraph();
    REQUIRE(navigationGraph.getNodes().size() == 2);

    std::vector<float> xs = nodeXsOnRow(tileMap, 80.0f);
    REQUIRE(xs == std::vector<float>{32.0f, 80.0f});

    REQUIRE(hasEdgeBetween(tileMap, 32.0f, 80.0f, 80.0f));
    REQUIRE(hasEdgeBetween(tileMap, 80.0f, 32.0f, 80.0f));

    for (const auto &edge : navigationGraph.getEdges())
        REQUIRE(edge.type == EdgeType::Walk);
}

TEST_CASE("No walk edge spans a gap between floors", "[TileMap][Navigation]")
{
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 0, 2);
    layFloor(tileMap, 5, 6, 9);
    tileMap.buildNavigationGraph();

    REQUIRE(!hasEdgeBetween(tileMap, 48.0f, 96.0f, 80.0f));
    REQUIRE(!hasEdgeBetween(tileMap, 96.0f, 48.0f, 80.0f));
    REQUIRE_FALSE(isReachable(tileMap, {48.0f, 80.0f}, {96.0f, 80.0f}));

    REQUIRE(isReachable(tileMap, {0.0f, 80.0f}, {48.0f, 80.0f}));
    REQUIRE(isReachable(tileMap, {96.0f, 80.0f}, {160.0f, 80.0f}));
}

TEST_CASE("No walk edge passes through a blocked tile", "[TileMap][Navigation]")
{
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 0, 5);
    tileMap.setTileIndex(glm::ivec2(3, 4), 1);
    tileMap.buildNavigationGraph();

    const NavigationGraph &navigationGraph = tileMap.getNavigationGraph();
    for (const auto &edge : navigationGraph.getEdges())
    {
        NavigationNode from = navigationGraph.getNode(edge.fromId);
        NavigationNode to = navigationGraph.getNode(edge.toId);
        float low = std::min(from.position.x, to.position.x);
        float high = std::max(from.position.x, to.position.x);
        REQUIRE_FALSE((low <= 48.0f && high >= 64.0f));
    }

    REQUIRE_FALSE(isReachable(tileMap, {0.0f, 80.0f}, {96.0f, 80.0f}));
}

TEST_CASE("Floors on different rows are not connected", "[TileMap][Navigation]")
{
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 0, 3);
    layFloor(tileMap, 8, 0, 3);
    tileMap.buildNavigationGraph();

    const NavigationGraph &navigationGraph = tileMap.getNavigationGraph();
    for (const auto &edge : navigationGraph.getEdges())
        REQUIRE(navigationGraph.getNode(edge.fromId).position.y ==
                navigationGraph.getNode(edge.toId).position.y);
}

TEST_CASE("Rebuilding the graph does not accumulate edges", "[TileMap][Navigation]")
{
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 0, 9);

    tileMap.buildNavigationGraph();
    size_t nodeCount = tileMap.getNavigationGraph().getNodes().size();
    size_t edgeCount = tileMap.getNavigationGraph().getEdges().size();

    tileMap.buildNavigationGraph();

    REQUIRE(tileMap.getNavigationGraph().getNodes().size() == nodeCount);
    REQUIRE(tileMap.getNavigationGraph().getEdges().size() == edgeCount);
}

TEST_CASE("The graph is built on load without being asked", "[TileMap][Navigation]")
{
    TileMapData tileMapData;
    tileMapData.size = 16;
    TilePalette palette = getDefaultTileDataMap();
    tileMapData.indices = std::vector<std::vector<int>>(10, std::vector<int>(10, 0));
    for (int x = 0; x < 10; ++x)
        (*tileMapData.indices)[5][x] = 1;

    TileMap tileMap(tileMapData, palettesFrom(palette));

    REQUIRE_FALSE(tileMap.getNavigationGraph().getNodes().empty());
    REQUIRE_FALSE(tileMap.getNavigationGraph().getEdges().empty());
}

TEST_CASE("A saved level carries no navigation data", "[TileMap][Navigation]")
{
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 0, 9);
    tileMap.buildNavigationGraph();

    REQUIRE_FALSE(tileMap.getNavigationGraph().getEdges().empty());

    std::string json;
    REQUIRE_FALSE(glz::write_json(tileMap.toTileMapData(), json));
    REQUIRE(json.find("navigation") == std::string::npos);
}

TEST_CASE("Every shipped level derives a graph from its tiles", "[TileMap][Navigation]")
{
    for (const auto &entry : std::filesystem::directory_iterator("../../assets/levels"))
    {
        if (entry.path().extension() != ".json")
            continue;

        TileMap tileMap(entry.path().string(), shippedPalettes());
        INFO("level " << entry.path().filename().string() << " has no walkable graph");
        REQUIRE_FALSE(tileMap.getNavigationGraph().getEdges().empty());
    }
}

TEST_CASE("Saving a level writes a readable grid and tile table, and reloads unchanged", "[TileMap][Level]")
{
    std::filesystem::path savePath =
        std::filesystem::temp_directory_path() / "platformer_save_roundtrip.json";
    std::filesystem::copy_file(
        "../../assets/levels/level6.json",
        savePath,
        std::filesystem::copy_options::overwrite_existing);

    TileMap loaded(savePath.string(), shippedPalettes());
    loaded.save();

    std::string savedJson = readFile(savePath);

    REQUIRE(savedJson.starts_with("{\n    \""));
    REQUIRE(savedJson.ends_with("\n}"));

    REQUIRE(savedJson.find("\"indices\":[\n        [") != std::string::npos);

    size_t rows = 0;
    for (size_t at = savedJson.find("\n        ["); at != std::string::npos; at = savedJson.find("\n        [", at + 1))
        ++rows;
    REQUIRE(rows == static_cast<size_t>(loaded.getHeight()));

    REQUIRE(savedJson.find("\"tilePalette\":\"default\"") != std::string::npos);
    REQUIRE(savedJson.find("\"tileData\"") == std::string::npos);

    REQUIRE(savedJson.find("\"playerStartTilePosition\":[") != std::string::npos);
    REQUIRE(savedJson.find("\"playerStartTilePosition\":[\n") == std::string::npos);

    TileMap reloaded(savePath.string(), shippedPalettes());
    reloaded.save();
    REQUIRE(readFile(savePath) == savedJson);

    REQUIRE(reloaded.getWidth() == loaded.getWidth());
    REQUIRE(reloaded.getHeight() == loaded.getHeight());
    REQUIRE(reloaded.getTileSize() == loaded.getTileSize());
    REQUIRE(reloaded.getNextLevel() == loaded.getNextLevel());
    REQUIRE(reloaded.getNpcs() == loaded.getNpcs());
    REQUIRE(reloaded.getPlayerStartWorldPosition() == loaded.getPlayerStartWorldPosition());
    REQUIRE(reloaded.getNavigationGraph().getNodes().size() ==
            loaded.getNavigationGraph().getNodes().size());
    REQUIRE(reloaded.getNavigationGraph().getEdges().size() ==
            loaded.getNavigationGraph().getEdges().size());

    for (int y = 0; y < loaded.getHeight(); ++y)
        for (int x = 0; x < loaded.getWidth(); ++x)
            REQUIRE(reloaded.tilePositionToTileIndex({x, y}) ==
                    loaded.tilePositionToTileIndex({x, y}));

    std::filesystem::remove(savePath);
}

TEST_CASE("Every shipped level is already in the format the editor saves", "[TileMap][Level]")
{
    for (const auto &entry : std::filesystem::directory_iterator("../../assets/levels"))
    {
        if (entry.path().extension() != ".json")
            continue;

        std::string original = readFile(entry.path());

        std::filesystem::path savePath =
            std::filesystem::temp_directory_path() / entry.path().filename();
        std::filesystem::copy_file(
            entry.path(),
            savePath,
            std::filesystem::copy_options::overwrite_existing);

        TileMap(savePath.string(), shippedPalettes()).save();

        INFO("level " << entry.path().filename().string() << " is not saved in the editor's format");
        REQUIRE(readFile(savePath) == original);

        std::filesystem::remove(savePath);
    }
}

TEST_CASE("A level knows where it heads next", "[TileMap][Level]")
{
    TileMap tileMap("../../assets/levels/level6.json", shippedPalettes());

    REQUIRE(tileMap.getNextLevel() == "../../assets/levels/level1.json");
    REQUIRE(tileMap.getNpcs().size() == 2);
    REQUIRE(tileMap.getNpcs()[0].type == "villager");
    REQUIRE(tileMap.getNavigationGraph().getNodes().size() == 8);
    REQUIRE_FALSE(tileMap.getNavigationGraph().getEdges().empty());

    for (const auto &entry : std::filesystem::directory_iterator("../../assets/levels"))
        if (entry.path().extension() == ".json")
            REQUIRE_FALSE(TileMap(entry.path().string(), shippedPalettes()).getNextLevel().empty());
}

TEST_CASE("A level naming a palette that does not exist fails to load", "[TileMap][Level]")
{
    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 4;
    tileMapData.height = 4;
    tileMapData.tilePalette = "nope";

    REQUIRE_THROWS_WITH(
        TileMap(tileMapData, shippedPalettes()),
        "Unknown tile palette \"nope\"");
}

TEST_CASE("Every shipped level uses a palette that exists", "[TileMap][Level]")
{
    for (const auto &entry : std::filesystem::directory_iterator("../../assets/levels"))
        if (entry.path().extension() == ".json")
            REQUIRE_NOTHROW(TileMap(entry.path().string(), shippedPalettes()));
}
