#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "test_helpers/test_tile_map_utils.hpp"
#include "physics/aabb.hpp"

TEST_CASE("TileMap initializes grid correctly", "[TileMap]")
{
    TileMap tileMap = setupTileMap();

    REQUIRE(tileMap.getWidth() == 10);
    REQUIRE(tileMap.getHeight() == 10);
    for (int tileY = 0; tileY < tileMap.getHeight(); ++tileY)
        for (int tileX = 0; tileX < tileMap.getWidth(); ++tileX)
            REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(tileX, tileY)) == 0);
    REQUIRE(tileMap.getTileSize() == 16);
}

TEST_CASE("TileMap set/get tile indices correctly", "[TileMap]")
{
    TileMap tileMap = setupTileMap();

    SECTION("Sets and gets tile indices correctly")
    {
        REQUIRE_NOTHROW(tileMap.setTileIndex(glm::ivec2(1, 1), 5));
        REQUIRE_NOTHROW(tileMap.setTileIndex(glm::ivec2(0, 2), 7));
        REQUIRE_NOTHROW(tileMap.setTileIndex(glm::ivec2(0, 0), 0));
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 5);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(0, 2)) == 7);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(2, 2)) == 0);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(0, 0)) == 0);
    }

    SECTION("tilePositionToTileIndex handles out of bounds by throwing out of range")
    {
        REQUIRE_THROWS_WITH(tileMap.tilePositionToTileIndex(glm::ivec2(-1, 0)), "Tile coordinates out of bounds");
        REQUIRE_THROWS_WITH(tileMap.tilePositionToTileIndex(glm::ivec2(0, -1)), "Tile coordinates out of bounds");
        REQUIRE_THROWS_WITH(tileMap.tilePositionToTileIndex(glm::ivec2(13, 0)), "Tile coordinates out of bounds");
        REQUIRE_THROWS_WITH(tileMap.tilePositionToTileIndex(glm::ivec2(0, 23)), "Tile coordinates out of bounds");
    }

    SECTION("setTileIndex handles out of bounds")
    {
        REQUIRE_THROWS_WITH(tileMap.setTileIndex(glm::ivec2(13, 0), 1), "Tile coordinates out of bounds");
    }

    SECTION("setTileIndex Throws on negative value")
    {
        REQUIRE_THROWS_WITH(tileMap.setTileIndex(glm::ivec2(2, 2), -5), "Tile index must be greater or equals to 0");
    }
}

TEST_CASE("TileMap returns correct tile", "[TileMap]")
{
    TileData solidTileData, emptyTileData;
    solidTileData.kind = TileKind::Solid;
    emptyTileData.kind = TileKind::Empty;
    TileMapData tileMapData;
    tileMapData.width = 3;
    tileMapData.height = 3;
    tileMapData.tileData = {{1, solidTileData},
                            {0, emptyTileData},
                            {3, emptyTileData}};
    TileMap tileMap(tileMapData);

    SECTION("Known indices")
    {
        const Tile &tile1 = tileMap.getTile(1);
        REQUIRE(tile1.getKind() == TileKind::Solid);
        REQUIRE_FALSE(tile1.isAnimated());

        const Tile &tile2 = tileMap.getTile(0);
        REQUIRE(tile2.getKind() == TileKind::Empty);
        REQUIRE_FALSE(tile2.isAnimated());

        const Tile &tile3 = tileMap.getTile(3);
        REQUIRE(tile3.getKind() == TileKind::Empty);
        REQUIRE_FALSE(tile3.isAnimated());
    }

    SECTION("Unknown indices")
    {
        REQUIRE_THROWS_WITH(tileMap.getTile(999), "Invalid tile index");
    }
}

TEST_CASE("TileMap animates tiles correctly", "[TileMap]")
{
    TileData animatedTileData1, animatedTileData2, emptyTileData;
    animatedTileData1.kind = animatedTileData2.kind = emptyTileData.kind = TileKind::Empty;
    animatedTileData1.animationData = {{{10, 11, 12}, 0.1f}};
    animatedTileData2.animationData = {{{5, 6}, 0.1f}};
    TileMapData tileMapData;
    tileMapData.width = 2;
    tileMapData.height = 2;
    tileMapData.tileData = {
        {1, animatedTileData1},
        {0, emptyTileData},
        {3, animatedTileData2}};
    TileMap tileMap(tileMapData);
    tileMap.setTileIndex(glm::ivec2(0, 0), 1);
    tileMap.setTileIndex(glm::ivec2(0, 1), 0);
    tileMap.setTileIndex(glm::ivec2(1, 1), 3);

    SECTION("Animated tiles")
    {
        const Tile &tile1 = tileMap.getTile(1);
        const Tile &tile2 = tileMap.getTile(3);
        REQUIRE(tile1.getCurrentFrame() == 10);
        REQUIRE(tile2.getCurrentFrame() == 5);
        tileMap.update(0.1f);
        REQUIRE(tile1.getCurrentFrame() == 11);
        REQUIRE(tile2.getCurrentFrame() == 6);
    }

    SECTION("Non-animated tiles")
    {
        const Tile &tile = tileMap.getTile(0);
        REQUIRE(tile.getCurrentFrame() == 0);
        tileMap.update(1.0f);
        REQUIRE(tile.getCurrentFrame() == 0);
    }
}

TEST_CASE("Pickup tile is defined correctly", "[TileMap]")
{
    TileData emptyTileData, pickupTileData;
    emptyTileData.kind = TileKind::Empty;
    pickupTileData.kind = TileKind::Pickup;
    pickupTileData.pickupReplaceIndex = 0;
    pickupTileData.animationData = std::nullopt;
    TileMapData tileMapData;
    tileMapData.width = 2;
    tileMapData.height = 2;
    tileMapData.tileData = {{0, emptyTileData},
                            {5, pickupTileData}};
    TileMap tileMap(tileMapData);
    tileMap.setTileIndex(glm::ivec2(1, 1), 5);
    const Tile &tile = tileMap.getTile(5);
    REQUIRE(tile.isPickup());
    REQUIRE(tile.getPickupReplaceIndex().value() == 0);
}

TEST_CASE("TileMap calculates world dimensions correctly", "[TileMap]")
{
    TileMap tileMap = setupTileMap();
    REQUIRE(tileMap.getWorldWidth() == 10 * 16);
    REQUIRE(tileMap.getWorldHeight() == 10 * 16);
}

TEST_CASE("TileMap worldToTilePositions returns correct tile coordinates", "[TileMap]")
{
    TileMap tileMap = setupTileMap();
    glm::vec2 worldPosition(15.0f, 15.0f);
    glm::vec2 size(16.0f, 16.0f);
    auto positions = tileMap.worldToTilePositions(worldPosition, size);
    std::vector<glm::ivec2> expected = {
        {0, 0}, {1, 0}, {0, 1}, {1, 1}};

    REQUIRE(positions == expected);
}

TEST_CASE("TileMap probeSolidTiles detects solid tile intersections", "[TileMap]")
{
    TileMap tileMap = setupTileMap(3, 3);
    tileMap.setTileIndex(glm::ivec2(1, 1), 1);

    AABB probeAABB(glm::vec2(16.0f, 16.0f), glm::vec2(16.0f));

    bool result = tileMap.probeSolidTiles(probeAABB, [](const AABB &_)
                                          { return true; });

    REQUIRE(result == true);
}