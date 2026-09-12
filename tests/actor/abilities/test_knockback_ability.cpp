#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/knockback_ability.hpp"
#include "actor/abilities/knockback_ability_data.hpp"
#include "actor/abilities/knockback_ability_state.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "combat/hit.hpp"
#include "helpers/actors.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/floor_level.hpp"
#include "helpers/palettes.hpp"
#include "helpers/player_fixtures.hpp"
#include "game/level.hpp"
#include "input/input_intentions.hpp"
#include "player/player.hpp"
#include "timing/fixed_time_step.hpp"

using Catch::Approx;

namespace
{
    constexpr float Step = 0.01f;

    Hit aHitPushing(float x, float y = 0.0f)
    {
        return Hit{1, glm::vec2(x, y), false};
    }
}

TEST_CASE("A push starts a knockback away from the hit, up and along", "[KnockbackAbility]")
{
    AbilityStates states;
    Observed observed;
    InputIntentions nothing;
    KnockbackAbilityData data;
    KnockbackAbility ability(data);
    observed.hits.push_back(Hit{1, glm::vec2(-1.0f, 0.0f), false});

    ability.decide(Step, nothing, observed, states);

    REQUIRE(states.knockback.active);
    REQUIRE(states.knockback.emit);
    REQUIRE(states.knockback.velocity.x == Approx(-data.speed));
    REQUIRE(states.knockback.velocity.y == Approx(data.lift));
}

TEST_CASE("A knockback says so once and lasts its duration", "[KnockbackAbility]")
{
    AbilityStates states;
    Observed observed;
    InputIntentions nothing;
    KnockbackAbilityData data;
    KnockbackAbility ability(data);
    observed.hits = {aHitPushing(1.0f)};

    ability.decide(Step, nothing, observed, states);
    observed.hits.clear();
    ability.decide(Step, nothing, observed, states);
    REQUIRE_FALSE(states.knockback.emit);
    REQUIRE(states.knockback.active);

    ability.decide(data.duration, nothing, observed, states);

    REQUIRE_FALSE(states.knockback.active);
    REQUIRE(states.knockback.velocity == glm::vec2(0.0f));
}

TEST_CASE("Nothing pushed, nothing moves", "[KnockbackAbility]")
{
    AbilityStates states;
    Observed observed;
    InputIntentions nothing;
    KnockbackAbility ability{KnockbackAbilityData{}};

    ability.decide(Step, nothing, observed, states);

    REQUIRE_FALSE(states.knockback.active);
    REQUIRE(states.knockback.velocity == glm::vec2(0.0f));
}

TEST_CASE("A push with no side to it keeps the last direction", "[KnockbackAbility]")
{
    AbilityStates states;
    Observed observed;
    InputIntentions nothing;
    KnockbackAbilityData data;
    KnockbackAbility ability(data);
    observed.hits.push_back(Hit{1, glm::vec2(-1.0f, 0.0f), false});
    ability.decide(Step, nothing, observed, states);
    ability.decide(data.duration, nothing, observed, states);

    observed.hits = {aHitPushing(0.0f)};
    ability.decide(Step, nothing, observed, states);

    REQUIRE(states.knockback.velocity.x == Approx(-data.speed));
}

TEST_CASE("A second push restarts the knockback", "[KnockbackAbility]")
{
    AbilityStates states;
    Observed observed;
    InputIntentions nothing;
    KnockbackAbilityData data;
    KnockbackAbility ability(data);
    observed.hits = {aHitPushing(1.0f)};
    ability.decide(data.duration * 0.5f, nothing, observed, states);

    observed.hits.push_back(Hit{1, glm::vec2(-1.0f, 0.0f), false});
    ability.decide(Step, nothing, observed, states);

    REQUIRE(states.knockback.velocity.x == Approx(-data.speed));
    REQUIRE(states.knockback.timeLeft == Approx(data.duration - Step));
}

TEST_CASE("Knockback data that cannot push is refused", "[KnockbackAbility]")
{
    REQUIRE_THROWS(KnockbackAbility(KnockbackAbilityData{0.0f, -100.0f, 0.1f}));
    REQUIRE_THROWS(KnockbackAbility(KnockbackAbilityData{100.0f, 10.0f, 0.1f}));
    REQUIRE_THROWS(KnockbackAbility(KnockbackAbilityData{100.0f, -100.0f, 0.0f}));
    REQUIRE_NOTHROW(KnockbackAbility(KnockbackAbilityData{100.0f, 0.0f, 0.1f}));
}

TEST_CASE("A hit that lands pushes the actor on its next step", "[KnockbackAbility]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    Level level(
        aFloorLevelPlacing({}),
        theOnlyPalette(aPaletteWithASolidTile()),
        playerDataWithHealth(3, 1.0f),
        {},
        {});
    player.standAt(feetOf(glm::ivec2(4, FloorLevelStanding)));
    FixedTimeStep timestepper;
    runFor(player, level, 0.1f, timestepper);
    REQUIRE(player.observed().velocity.x == 0.0f);

    player.takeHit(aHitPushing(1.0f));
    runFor(player, level, Step, timestepper);

    REQUIRE(player.observed().velocity.x > 0.0f);
    REQUIRE(player.observed().velocity.y < 0.0f);
}

TEST_CASE("A hit is observed for one step, so it pushes once", "[KnockbackAbility]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    Level level(
        aFloorLevelPlacing({}),
        theOnlyPalette(aPaletteWithASolidTile()),
        playerDataWithHealth(3, 1.0f),
        {},
        {});
    player.standAt(feetOf(glm::ivec2(4, FloorLevelStanding)));
    FixedTimeStep timestepper;
    runFor(player, level, 0.1f, timestepper);
    player.takeHit(aHitPushing(1.0f));

    int starts = 0;
    for (int step = 0; step < 10; ++step)
    {
        runFor(player, level, Step, timestepper);
        if (player.abilityStates().knockback.emit)
            ++starts;
    }

    REQUIRE(starts == 1);
}

TEST_CASE("A lethal hit does not push a corpse", "[KnockbackAbility]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    Level level(
        aFloorLevelPlacing({}),
        theOnlyPalette(aPaletteWithASolidTile()),
        playerDataWithHealth(3, 1.0f),
        {},
        {});
    player.standAt(feetOf(glm::ivec2(4, FloorLevelStanding)));
    FixedTimeStep timestepper;
    runFor(player, level, 0.1f, timestepper);

    player.takeHit(lethalHit());
    runFor(player, level, Step, timestepper);

    REQUIRE_FALSE(player.abilityStates().knockback.active);
}

TEST_CASE("A knocked back actor keeps facing the way it was", "[KnockbackAbility]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    Level level(
        aFloorLevelPlacing({}),
        theOnlyPalette(aPaletteWithASolidTile()),
        playerDataWithHealth(3, 1.0f),
        {},
        {});
    player.standAt(feetOf(glm::ivec2(4, FloorLevelStanding)));
    FixedTimeStep timestepper;
    runFor(player, level, 0.1f, timestepper);
    bool facedLeft = player.observed().facingLeft;

    player.takeHit(aHitPushing(facedLeft ? 1.0f : -1.0f));
    runFor(player, level, 0.05f, timestepper);

    REQUIRE(player.observed().facingLeft == facedLeft);
}
