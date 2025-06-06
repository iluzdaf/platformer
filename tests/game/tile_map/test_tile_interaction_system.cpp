

#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/tile_map/tile_interaction_system.hpp"
#include "game/player/player.hpp"
#include "game/tile_map/tile_map.hpp"

namespace
{
    TileMap setupTileMapWithPickup()
    {
        TileMap tileMap(10, 10, 16);
        TileData pickupTile;
        pickupTile.kind = TileKind::Pickup;
        pickupTile.pickupReplaceIndex = 1;
        tileMap.setTileData(2, pickupTile);
        tileMap.setTileIndex({1, 1}, 2);
        return tileMap;
    }

    TileMap setupTileMapWithSpikes()
    {
        TileMap tileMap(10, 10, 16);
        TileData spikeTile;
        spikeTile.kind = TileKind::Spikes;
        tileMap.setTileData(3, spikeTile);
        tileMap.setTileIndex({1, 1}, 3);
        return tileMap;
    }
}

TEST_CASE("TileInteractionSystem triggers level complete on pickup")
{
    TileMap tileMap = setupTileMapWithPickup();
    Player player;
    player.setPosition(glm::vec2(1 * 16, 1 * 16));
    TileInteractionSystem system;

    bool completed = false;
    player.onLevelComplete.connect([&]()
                                   { completed = true; });

    system.fixedUpdate(player, tileMap);
    REQUIRE(completed);
    REQUIRE(tileMap.getTileIndex({1, 1}) == 1);
}

TEST_CASE("TileInteractionSystem triggers death on spikes")
{
    TileMap tileMap = setupTileMapWithSpikes();
    Player player;
    player.setPosition(glm::vec2(1 * 16, 1 * 16));
    TileInteractionSystem system;

    bool died = false;
    player.onDeath.connect([&]()
                           { died = true; });

    system.fixedUpdate(player, tileMap);
    REQUIRE(died);
}

TEST_CASE("TileInteractionSystem replaces pickup tile with replacement index")
{
    TileMap tileMap = setupTileMapWithPickup();
    Player player;
    player.setPosition(glm::vec2(1 * 16, 1 * 16));
    TileInteractionSystem system;

    system.fixedUpdate(player, tileMap);

    REQUIRE(tileMap.getTileIndex({1, 1}) == 1);
}