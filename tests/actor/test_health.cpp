#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/health.hpp"
#include "actor/health_data.hpp"
#include "actor/hit.hpp"
#include "helpers/actors.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "game/level.hpp"
#include "timing/fixed_time_step.hpp"
#include "helpers/player_fixtures.hpp"
#include "helpers/levels.hpp"
#include "helpers/palettes.hpp"
#include "helpers/npc_fixtures.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include <optional>

namespace
{
    Hit aHitOf(int damage)
    {
        return Hit{damage, glm::vec2(0.0f), false};
    }

    Health threePoints(float invulnerableFor = 0.0f)
    {
        return Health(HealthData{3, invulnerableFor});
    }
}

TEST_CASE("Health starts full and alive", "[Health]")
{
    Health health = threePoints();

    REQUIRE(health.points() == 3);
    REQUIRE(health.maximum() == 3);
    REQUIRE(health.alive());
    REQUIRE_FALSE(health.invulnerable());
}

TEST_CASE("A hit takes its damage and lands", "[Health]")
{
    Health health = threePoints();

    REQUIRE(health.takeHit(aHitOf(2)));

    REQUIRE(health.points() == 1);
    REQUIRE(health.alive());
}

TEST_CASE("Damage past the last point leaves none, not a debt", "[Health]")
{
    Health health = threePoints();

    health.takeHit(aHitOf(5));

    REQUIRE(health.points() == 0);
    REQUIRE_FALSE(health.alive());
}

TEST_CASE("A lethal hit kills whatever is left", "[Health]")
{
    Health health = threePoints();

    REQUIRE(health.takeHit(lethalHit()));

    REQUIRE_FALSE(health.alive());
}

TEST_CASE("Nothing lands on the dead", "[Health]")
{
    Health health = threePoints();
    health.takeHit(lethalHit());

    REQUIRE_FALSE(health.takeHit(aHitOf(1)));
    REQUIRE_FALSE(health.takeHit(lethalHit()));
}

TEST_CASE("A hit opens an invulnerable window that refuses the next", "[Health]")
{
    Health health = threePoints(0.5f);

    REQUIRE(health.takeHit(aHitOf(1)));
    REQUIRE(health.invulnerable());
    REQUIRE_FALSE(health.takeHit(aHitOf(1)));
    REQUIRE(health.points() == 2);
}

TEST_CASE("The invulnerable window closes with time", "[Health]")
{
    Health health = threePoints(0.5f);
    health.takeHit(aHitOf(1));

    health.update(0.25f);
    REQUIRE(health.invulnerable());
    health.update(0.25f);
    REQUIRE_FALSE(health.invulnerable());

    REQUIRE(health.takeHit(aHitOf(1)));
    REQUIRE(health.points() == 1);
}

TEST_CASE("A lethal hit ignores the invulnerable window", "[Health]")
{
    Health health = threePoints(1.0f);
    health.takeHit(aHitOf(1));

    REQUIRE(health.takeHit(lethalHit()));
    REQUIRE_FALSE(health.alive());
}

TEST_CASE("Without a window every hit lands", "[Health]")
{
    Health health = threePoints();

    REQUIRE(health.takeHit(aHitOf(1)));
    REQUIRE(health.takeHit(aHitOf(1)));
    REQUIRE(health.points() == 1);
}

TEST_CASE("Health remembers the last hit that landed, not the ones refused", "[Health]")
{
    Health health = threePoints(1.0f);
    REQUIRE_FALSE(health.lastHit());

    health.takeHit(Hit{1, glm::vec2(-1.0f, 0.0f), false});
    health.takeHit(Hit{1, glm::vec2(1.0f, 0.0f), false});

    REQUIRE(health.lastHit());
    REQUIRE(health.lastHit()->direction.x == -1.0f);
}

TEST_CASE("Health of less than a point, or a negative window, is refused", "[Health]")
{
    REQUIRE_THROWS(Health(HealthData{0, 0.0f}));
    REQUIRE_THROWS(Health(HealthData{1, -1.0f}));
}

TEST_CASE("An actor is alive with one point until a hit says otherwise", "[Health]")
{
    Player player = aPlayerWithEveryAbility();

    REQUIRE(player.alive());
    REQUIRE(player.health().maximum() == 1);

    player.takeHit(aHitOf(1));

    REQUIRE_FALSE(player.alive());
}

TEST_CASE("A player says hurt while alive and dead once, whatever keeps hitting", "[Health]")
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

TEST_CASE("An npc says hurt and dead the same way the player does", "[Health]")
{
    NpcData npcData = setupNpcData();
    npcData.actorData.healthData = HealthData{2, 0.0f};
    Npc npc(NpcSpawnData{"villager", glm::vec2(0.0f), std::nullopt}, npcData);
    int hurts = 0, deaths = 0;
    npc.onHurt.connect([&] { ++hurts; });
    npc.onDeath.connect([&] { ++deaths; });

    npc.takeHit(aHitOf(1));
    npc.takeHit(aHitOf(1));
    npc.takeHit(aHitOf(1));

    REQUIRE(hurts == 1);
    REQUIRE(deaths == 1);
}

TEST_CASE("A player's invulnerable window runs on the fixed step", "[Health]")
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
