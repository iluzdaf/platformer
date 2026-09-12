#include <glm/gtc/matrix_transform.hpp>
#include <catch2/catch_test_macros.hpp>
#include "combat/hit.hpp"
#include "game/level.hpp"
#include "helpers/actors.hpp"
#include "helpers/floor_level.hpp"
#include "helpers/palettes.hpp"
#include "helpers/player_fixtures.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "timing/fixed_time_step.hpp"

namespace
{
    Hit aHitOf(int damage)
    {
        return Hit{damage, glm::vec2(0.0f), false};
    }
}

TEST_CASE("An actor is alive with one point until a hit says otherwise", "[TakingHits]")
{
    Player player = aPlayerWithEveryAbility();

    REQUIRE(player.alive());
    REQUIRE(player.health().maximum() == 1);

    player.takeHit(aHitOf(1));

    REQUIRE_FALSE(player.alive());
}

TEST_CASE("An actor says hurt while alive and dead once, whatever keeps hitting", "[TakingHits]")
{
    Player player(playerDataWithHealth(2, 0.0f), noIntentions());
    int hurts = 0, deaths = 0;
    player.onHurt.connect([&] { ++hurts; });
    player.onDeath.connect([&] { ++deaths; });

    player.takeHit(aHitOf(1));
    player.takeHit(aHitOf(1));
    player.takeHit(aHitOf(1));
    player.takeHit(lethalHit());

    REQUIRE(hurts == 1);
    REQUIRE(deaths == 1);
}

TEST_CASE("A player's invulnerable window runs on the fixed step", "[TakingHits]")
{
    PlayerData playerData = playerDataWithHealth(3, 0.2f);
    Player player(playerData, noIntentions());
    player.takeHit(aHitOf(1));
    REQUIRE(player.health().invulnerable());

    Level level(
        aFloorLevelPlacing({}), theOnlyPalette(aPaletteWithASolidTile()), playerData, {}, {});
    FixedTimeStep timestepper;
    runFor(player, level, 0.3f, timestepper);

    REQUIRE_FALSE(player.health().invulnerable());
}
