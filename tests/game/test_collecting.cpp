#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "game/scoring_system.hpp"
#include "tile_map/tile_interaction_system.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette.hpp"
#include "tile_map/tile_pickup_data.hpp"
#include "test_helpers/test_player_utils.hpp"
#include "test_helpers/test_tile_map_utils.hpp"

namespace
{
    constexpr glm::ivec2 WhereThePlayerStands{1, 1};

    std::optional<int> aTileWorthCollecting(const TilePalette &palette)
    {
        for (const auto &[tileIndex, tileData] : palette.tiles)
            if (tileData.pickup)
                return tileIndex;

        return std::nullopt;
    }
}

TEST_CASE("Every pickup the shipped palettes offer is worth collecting", "[Collecting]")
{
    bool anyPickup = false;
    for (const auto &[name, palette] : shippedPalettes())
        for (const auto &[tileIndex, tileData] : palette.tiles)
            if (tileData.pickup)
            {
                anyPickup = true;
                REQUIRE(tileData.pickup->scoreDelta);
                REQUIRE(*tileData.pickup->scoreDelta > 0);
            }

    REQUIRE(anyPickup);
}

TEST_CASE("Collecting the shipped coin adds ten to the score", "[Collecting]")
{
    const TilePalette &palette = shippedPalettes().at("default");
    std::optional<int> coin = aTileWorthCollecting(palette);
    REQUIRE(coin);

    TileMap tileMap = setupTileMap(10, 10, 16, palette);
    tileMap.setTileIndex(WhereThePlayerStands, *coin);

    Player player = setupPlayer();
    player.setPosition(tileMap.topLeftOfTile(WhereThePlayerStands));

    ScoringSystem scoringSystem;
    player.onPickup.connect([&](int scoreDelta) { scoringSystem.addScore(scoreDelta); });

    TileInteractionSystem interactions;
    interactions.fixedUpdate(player, tileMap);

    REQUIRE(scoringSystem.getScore() == 10);
}
