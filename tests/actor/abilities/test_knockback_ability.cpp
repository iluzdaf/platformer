#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/knockback_ability.hpp"
#include "actor/abilities/knockback_ability_data.hpp"
#include "actor/abilities/knockback_ability_state.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/observed.hpp"
#include "actor/hit.hpp"
#include "helpers/actors.hpp"
#include "helpers/levels.hpp"
#include "helpers/palettes.hpp"
#include "helpers/player_fixtures.hpp"
#include "helpers/tiles.hpp"
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
    ActorMotionState state;
    Observed observed;
    InputIntentions nothing;
    KnockbackAbilityData data;
    KnockbackAbility ability(data);
    observed.hits.push_back(Hit{1, glm::vec2(-1.0f, 0.0f), false});

    ability.applyMovement(Step, nothing, observed, state);

    REQUIRE(state.knockback.active);
    REQUIRE(state.knockback.emit);
    REQUIRE(state.knockback.velocity.x == Approx(-data.speed));
    REQUIRE(state.knockback.velocity.y == Approx(data.lift));
}

TEST_CASE("A knockback says so once and lasts its duration", "[KnockbackAbility]")
{
    ActorMotionState state;
    Observed observed;
    InputIntentions nothing;
    KnockbackAbilityData data;
    KnockbackAbility ability(data);
    observed.hits = {aHitPushing(1.0f)};

    ability.applyMovement(Step, nothing, observed, state);
    observed.hits.clear();
    ability.applyMovement(Step, nothing, observed, state);
    REQUIRE_FALSE(state.knockback.emit);
    REQUIRE(state.knockback.active);

    ability.applyMovement(data.duration, nothing, observed, state);

    REQUIRE_FALSE(state.knockback.active);
    REQUIRE(state.knockback.velocity == glm::vec2(0.0f));
}

TEST_CASE("Nothing pushed, nothing moves", "[KnockbackAbility]")
{
    ActorMotionState state;
    Observed observed;
    InputIntentions nothing;
    KnockbackAbility ability{KnockbackAbilityData{}};

    ability.applyMovement(Step, nothing, observed, state);

    REQUIRE_FALSE(state.knockback.active);
    REQUIRE(state.knockback.velocity == glm::vec2(0.0f));
}

TEST_CASE("A push with no side to it keeps the last direction", "[KnockbackAbility]")
{
    ActorMotionState state;
    Observed observed;
    InputIntentions nothing;
    KnockbackAbilityData data;
    KnockbackAbility ability(data);
    observed.hits.push_back(Hit{1, glm::vec2(-1.0f, 0.0f), false});
    ability.applyMovement(Step, nothing, observed, state);
    ability.applyMovement(data.duration, nothing, observed, state);

    observed.hits = {aHitPushing(0.0f)};
    ability.applyMovement(Step, nothing, observed, state);

    REQUIRE(state.knockback.velocity.x == Approx(-data.speed));
}

TEST_CASE("A second push restarts the knockback", "[KnockbackAbility]")
{
    ActorMotionState state;
    Observed observed;
    InputIntentions nothing;
    KnockbackAbilityData data;
    KnockbackAbility ability(data);
    observed.hits = {aHitPushing(1.0f)};
    ability.applyMovement(data.duration * 0.5f, nothing, observed, state);

    observed.hits.push_back(Hit{1, glm::vec2(-1.0f, 0.0f), false});
    ability.applyMovement(Step, nothing, observed, state);

    REQUIRE(state.knockback.velocity.x == Approx(-data.speed));
    REQUIRE(state.knockback.timeLeft == Approx(data.duration - Step));
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
        if (player.motion().knockback.emit)
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

    REQUIRE_FALSE(player.motion().knockback.active);
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
    bool facedLeft = player.state().facingLeft;

    player.takeHit(aHitPushing(facedLeft ? 1.0f : -1.0f));
    runFor(player, level, 0.05f, timestepper);

    REQUIRE(player.state().facingLeft == facedLeft);
}
