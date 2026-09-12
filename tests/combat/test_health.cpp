#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "combat/health.hpp"
#include "combat/health_data.hpp"
#include "combat/hit.hpp"

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
    REQUIRE_THROWS_WITH(
        Health(HealthData{0, 0.0f}),
        Catch::Matchers::ContainsSubstring("A health of less than one point is nobody alive"));
    REQUIRE_THROWS_WITH(
        Health(HealthData{1, -1.0f}),
        Catch::Matchers::ContainsSubstring("An invulnerable window cannot be negative"));
}
