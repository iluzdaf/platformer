#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "tile_map/tile_data.hpp"
#include "tile_map/touching_tiles.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "helpers/actors.hpp"
#include "helpers/levels.hpp"
#include "helpers/player_fixtures.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "physics/aabb.hpp"
#include "physics/physics_body.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "timing/fixed_time_step.hpp"

TEST_CASE("Spikes", "[TouchingTiles]")
{
    TileData spikeTileData;
    spikeTileData.deadly = true;
    TileMap tileMap = aTileMap({{{1, 1}, 3}}, 10, 10, 16, paletteOf({{3, spikeTileData}}));
    Player player = aPlayerWithEveryAbility();
    player.standAt(feetOf(glm::ivec2(1, 1)));

    SECTION("Triggers onDeath")
    {
        bool died = false;
        player.onDeath.connect([&]() { died = true; });
        touchTiles(player, tileMap);
        REQUIRE(died);
    }

    SECTION("Does not replace")
    {
        touchTiles(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 3);
    }
}

TEST_CASE("Empty", "[TouchingTiles]")
{
    TileData emptyTileData;
    TileMap tileMap = aTileMap({{{1, 1}, 0}}, 10, 10, 16, paletteOf({{0, emptyTileData}}));
    Player player = aPlayerWithEveryAbility();
    player.standAt(feetOf(glm::ivec2(1, 1)));

    SECTION("Does not replace")
    {
        touchTiles(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 0);
    }
}

TEST_CASE("Portal", "[TouchingTiles]")
{
    TileData portalTileData;
    portalTileData.portal = true;
    TileMap tileMap = aTileMap({{{1, 1}, 4}}, 10, 10, 16, paletteOf({{4, portalTileData}}));
    Player player = aPlayerWithEveryAbility();
    player.standAt(feetOf(glm::ivec2(1, 1)));

    SECTION("Triggers onLevelComplete")
    {
        bool completed = false;
        player.onLevelComplete.connect([&] { completed = true; });
        touchTiles(player, tileMap);
        REQUIRE(completed);
    }

    SECTION("Does not replace")
    {
        touchTiles(player, tileMap);
        REQUIRE(tileMap.tilePositionToTileIndex(glm::ivec2(1, 1)) == 4);
    }
}
namespace
{
    constexpr int Electrified = 1;
    constexpr int Plain = 2;

    TilePaletteData wallsThatMayKill()
    {
        TileData electrified;
        electrified.solid = electrified.deadly = true;
        TileData plain;
        plain.solid = true;
        return paletteOf({{0, TileData{}}, {Electrified, electrified}, {Plain, plain}});
    }

    void restAgainst(Player &player, glm::ivec2 tile, glm::vec2 towards)
    {
        glm::vec2 collider = player.body().colliderSize();
        glm::vec2 colliderTopLeft = topLeftOf(tile) - towards * collider;
        player.standAt(colliderTopLeft + glm::vec2(collider.x * 0.5f, collider.y));
    }

    bool diesTouching(Player &player, const TileMap &tileMap)
    {
        bool died = false;
        player.onDeath.connect([&] { died = true; });
        touchTiles(player, tileMap);
        return died;
    }
}

TEST_CASE("A player standing on an electrified floor dies", "[TouchingTiles]")
{
    TileMap tileMap = aTileMap({{{1, 5}, Electrified}}, 10, 10, 16, wallsThatMayKill());
    Player player = aPlayerWithEveryAbility();
    restAgainst(player, glm::ivec2(1, 5), glm::vec2(0.0f, 1.0f));

    REQUIRE(player.body().aabb().bottom() == topLeftOf({1, 5}).y);
    REQUIRE(diesTouching(player, tileMap));
}

TEST_CASE("A player standing on a plain floor lives", "[TouchingTiles]")
{
    TileMap tileMap = aTileMap({{{1, 5}, Plain}}, 10, 10, 16, wallsThatMayKill());
    Player player = aPlayerWithEveryAbility();
    restAgainst(player, glm::ivec2(1, 5), glm::vec2(0.0f, 1.0f));

    REQUIRE_FALSE(diesTouching(player, tileMap));
}

TEST_CASE("A player pressed against an electrified wall dies", "[TouchingTiles]")
{
    TileMap tileMap = aTileMap({{{2, 4}, Electrified}}, 10, 10, 16, wallsThatMayKill());
    Player player = aPlayerWithEveryAbility();
    restAgainst(player, glm::ivec2(2, 4), glm::vec2(1.0f, 0.0f));

    REQUIRE(player.body().aabb().right() == topLeftOf({2, 4}).x);
    REQUIRE(diesTouching(player, tileMap));
}

TEST_CASE("A player half a pixel short of an electrified wall lives", "[TouchingTiles]")
{
    TileMap tileMap = aTileMap({{{2, 4}, Electrified}}, 10, 10, 16, wallsThatMayKill());
    Player player = aPlayerWithEveryAbility();
    restAgainst(player, glm::ivec2(2, 4), glm::vec2(1.0f, 0.0f));
    player.standAt(player.feet() - glm::vec2(0.5f, 0.0f));

    REQUIRE_FALSE(diesTouching(player, tileMap));
}

TEST_CASE(
    "Physics leaves a player resting on an electrified floor close enough to die",
    "[TouchingTiles]")
{
    LevelData levelData = aFloorLevelPlacing({}, Electrified);
    Level level(
        levelData, theOnlyPalette(wallsThatMayKill()), playerDataWithEveryAbility(), {}, {});
    Player player = aPlayerWithEveryAbility();
    player.standAt(levelData.playerFeet - glm::vec2(0.0f, 40.0f));
    FixedTimeStep timestepper;

    runFor(player, level, 1.0f, timestepper);

    REQUIRE(player.body().aabb().bottom() == topLeftOf({1, FloorLevelRow}).y);
    REQUIRE(diesTouching(player, level.getTileMap()));
}
