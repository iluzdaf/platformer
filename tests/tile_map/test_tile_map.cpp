#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <filesystem>
#include <string>
#include <glaze/glaze.hpp>
#include <optional>
#include <vector>
#include "test_helpers/asset_path.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "physics/aabb.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_pickup_data.hpp"
#include "tile_map/tile_map_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "tile_map/tile.hpp"

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
        REQUIRE_THROWS_WITH(
            tileMap.tilePositionToTileIndex(glm::ivec2(-1, 0)), "Tile coordinates out of bounds");
        REQUIRE_THROWS_WITH(
            tileMap.tilePositionToTileIndex(glm::ivec2(0, -1)), "Tile coordinates out of bounds");
        REQUIRE_THROWS_WITH(
            tileMap.tilePositionToTileIndex(glm::ivec2(13, 0)), "Tile coordinates out of bounds");
        REQUIRE_THROWS_WITH(
            tileMap.tilePositionToTileIndex(glm::ivec2(0, 23)), "Tile coordinates out of bounds");
    }

    SECTION("setTileIndex handles out of bounds")
    {
        REQUIRE_THROWS_WITH(
            tileMap.setTileIndex(glm::ivec2(13, 0), 1), "Tile coordinates out of bounds");
    }

    SECTION("setTileIndex Throws on negative value")
    {
        REQUIRE_THROWS_WITH(
            tileMap.setTileIndex(glm::ivec2(2, 2), -5),
            "Tile index must be greater or equals to 0");
    }
}

TEST_CASE("TileMap returns correct tile", "[TileMap]")
{
    TileData solidTileData, emptyTileData;
    solidTileData.solid = true;
    TileMapData tileMapData;
    tileMapData.width = 3;
    tileMapData.height = 3;
    TilePalette palette = paletteOf({{1, solidTileData}, {0, emptyTileData}, {3, emptyTileData}});
    TileMap tileMap(tileMapData, palettesFrom(palette));

    SECTION("Known indices")
    {
        const Tile &tile1 = tileMap.getTile(1);
        REQUIRE(tile1.isSolid());
        REQUIRE_FALSE(tile1.isAnimated());

        const Tile &tile2 = tileMap.getTile(0);
        REQUIRE(tile2.isEmpty());
        REQUIRE_FALSE(tile2.isAnimated());

        const Tile &tile3 = tileMap.getTile(3);
        REQUIRE(tile3.isEmpty());
        REQUIRE_FALSE(tile3.isAnimated());
    }

    SECTION("Unknown indices")
    {
        REQUIRE_THROWS_WITH(tileMap.getTile(999), "Invalid tile index");
    }
}

TEST_CASE("A level is drawn from the tile set its palette names", "[TileMap]")
{
    TilePalette palette = paletteOf({{0, TileData{}}});
    palette.tileSet.texture = "textures/somewhere_else.png";
    palette.tileSet.tileSize = 8;

    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 2;
    tileMapData.height = 2;

    TileMap tileMap(tileMapData, palettesFrom(palette));

    REQUIRE(tileMap.getTileSet().texture == "textures/somewhere_else.png");
    REQUIRE(tileMap.getTileSet().tileSize == 8);
}

TEST_CASE("A tile set cell is not the world size of a tile", "[TileMap]")
{
    TilePalette palette = paletteOf({{0, TileData{}}});
    palette.tileSet.tileSize = 8;

    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.width = 2;
    tileMapData.height = 2;

    TileMap tileMap(tileMapData, palettesFrom(palette));

    REQUIRE(tileMap.getTileSize() == 16);
    REQUIRE(tileMap.getTileSet().tileSize == 8);
}

TEST_CASE("Every shipped palette names a texture that is on disk", "[TileMap]")
{
    for (const auto &[name, palette] : shippedPalettes())
        REQUIRE(std::filesystem::exists(assetPath(palette.tileSet.texture)));
}

TEST_CASE("A palette survives being written and read back", "[TilePalette]")
{
    std::string written;
    REQUIRE_FALSE(glz::write_json(shippedPalettes(), written));

    TilePalettes readBack;
    REQUIRE_FALSE(glz::read_json(readBack, written));

    std::string rewritten;
    REQUIRE_FALSE(glz::write_json(readBack, rewritten));

    REQUIRE(rewritten == written);
    REQUIRE(readBack.at("default").tileSet == shippedPalettes().at("default").tileSet);
    REQUIRE(readBack.at("default").tiles.size() == shippedPalettes().at("default").tiles.size());
}

TEST_CASE("A painted tile the palette says nothing about is empty", "[TileMap]")
{
    TilePalette palette = paletteOf({{0, TileData{}}, {1, TileData{}}});

    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.indices = std::vector<std::vector<int>>{{0, 1}, {1, 7}};

    TileMap tileMap(tileMapData, palettesFrom(palette));

    const Tile &tile = tileMap.getTile(7);
    REQUIRE(tile.isEmpty());
    REQUIRE_FALSE(tile.isSolid());
    REQUIRE(tile.getCurrentFrame() == 7);
}

TEST_CASE("A tile painted after loading is remembered", "[TileMap]")
{
    TileMap tileMap = setupTileMap(3, 3);

    tileMap.setTileIndex(glm::ivec2(1, 1), 24);

    REQUIRE(tileMap.getTile(24).isEmpty());
    REQUIRE(tileMap.getTile(24).getCurrentFrame() == 24);
}

TEST_CASE("A level painted only with tiles its palette has loads", "[TileMap]")
{
    TilePalette palette = paletteOf({{0, TileData{}}, {1, TileData{}}});

    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.indices = std::vector<std::vector<int>>{{0, 1}, {1, 0}};

    REQUIRE_NOTHROW(TileMap(tileMapData, palettesFrom(palette)));
}

TEST_CASE("The empty tile is paintable even when a palette omits it", "[TileMap]")
{
    TilePalette palette = paletteOf({{1, TileData{}}});

    TileMapData tileMapData;
    tileMapData.size = 16;
    tileMapData.indices = std::vector<std::vector<int>>{{0, 1}, {1, 0}};

    REQUIRE_NOTHROW(TileMap(tileMapData, palettesFrom(palette)));
}

TEST_CASE("TileMap animates tiles correctly", "[TileMap]")
{
    TileData animatedTileData1, animatedTileData2, emptyTileData;
    animatedTileData1.animationData = {{{10, 11, 12}, 0.1f}};
    animatedTileData2.animationData = {{{5, 6}, 0.1f}};
    TileMapData tileMapData;
    tileMapData.width = 2;
    tileMapData.height = 2;
    TilePalette palette =
        paletteOf({{1, animatedTileData1}, {0, emptyTileData}, {3, animatedTileData2}});
    TileMap tileMap(tileMapData, palettesFrom(palette));
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
    pickupTileData.pickup = TilePickupData{0, std::nullopt};
    pickupTileData.animationData = std::nullopt;
    TileMapData tileMapData;
    tileMapData.width = 2;
    tileMapData.height = 2;
    TilePalette palette = paletteOf({{0, emptyTileData}, {5, pickupTileData}});
    TileMap tileMap(tileMapData, palettesFrom(palette));
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

TEST_CASE("TileMap tilesOverlapping returns correct tile coordinates", "[TileMap]")
{
    TileMap tileMap = setupTileMap();
    glm::vec2 worldPosition(15.0f, 15.0f);
    glm::vec2 size(16.0f, 16.0f);
    auto positions = tileMap.tilesOverlapping(worldPosition, size);
    std::vector<glm::ivec2> expected = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};

    REQUIRE(positions == expected);
}

TEST_CASE("TileMap probeSolidTiles detects solid tile intersections", "[TileMap]")
{
    TileMap tileMap = setupTileMap(3, 3);
    tileMap.setTileIndex(glm::ivec2(1, 1), 1);

    AABB probeAABB(glm::vec2(16.0f, 16.0f), glm::vec2(16.0f));

    bool result =
        tileMap.probeSolidTiles(probeAABB, [](const Tile &, const AABB &) { return true; });

    REQUIRE(result == true);
}

TEST_CASE("TileMap names the spot an actor stands on inside a tile", "[TileMap]")
{
    TileMap tileMap = setupTileMap();

    glm::vec2 standingIn = tileMap.feetOnTile(glm::ivec2(3, 4));

    REQUIRE(standingIn.x == 3 * 16.0f + 8.0f);
    REQUIRE(standingIn.y == 5 * 16.0f);
}

TEST_CASE("The two corners of a tile say which corner they are", "[TileMap]")
{
    TileMap tileMap = setupTileMap();
    glm::ivec2 tilePosition(3, 4);
    float tileSize = static_cast<float>(tileMap.getTileSize());

    glm::vec2 corner = tileMap.topLeftOfTile(tilePosition);
    glm::vec2 feet = tileMap.feetOnTile(tilePosition);

    REQUIRE(feet - corner == glm::vec2(tileSize * 0.5f, tileSize));
}

TEST_CASE("The tile containing a point is not the tile stood on at it", "[TileMap]")
{
    TileMap tileMap = setupTileMap();
    glm::vec2 feet = tileMap.feetOnTile(glm::ivec2(3, 4));

    REQUIRE(tileMap.tileStoodOnAt(feet) == glm::ivec2(3, 4));
    REQUIRE(tileMap.tileContaining(feet) == glm::ivec2(3, 5));
}

TEST_CASE("TileMap names the tile an actor standing somewhere is on", "[TileMap]")
{
    TileMap tileMap = setupTileMap();
    glm::ivec2 tilePosition(3, 4);

    REQUIRE(tileMap.tileStoodOnAt(tileMap.feetOnTile(tilePosition)) == tilePosition);
}

TEST_CASE("A spot on a tile's edge belongs to the tile it is the edge of", "[TileMap]")
{
    TileMap tileMap = setupTileMap();
    float rightEdge = static_cast<float>(tileMap.getWidth() * tileMap.getTileSize());
    float floorSurface = static_cast<float>(tileMap.getHeight() * tileMap.getTileSize());

    glm::ivec2 corner = tileMap.tileStoodOnAt(glm::vec2(rightEdge, floorSurface));

    REQUIRE(tileMap.validTilePosition(corner));
    REQUIRE(corner == glm::ivec2(tileMap.getWidth() - 1, tileMap.getHeight() - 1));
}

TEST_CASE("Nothing stands on ground it is buried in", "[TileMap]")
{
    TileMap tileMap = setupTileMap();
    tileMap.setTileIndex(glm::ivec2(3, 5), 1);

    REQUIRE(tileMap.standsOnGround(glm::ivec2(3, 4)));

    tileMap.setTileIndex(glm::ivec2(3, 4), 1);

    REQUIRE_FALSE(tileMap.standsOnGround(glm::ivec2(3, 4)));
}

TEST_CASE("Nothing stands on thin air", "[TileMap]")
{
    TileMap tileMap = setupTileMap();

    REQUIRE_FALSE(tileMap.standsOnGround(glm::ivec2(3, 4)));
}
