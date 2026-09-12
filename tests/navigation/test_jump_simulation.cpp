#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <algorithm>
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_build_report.hpp"
#include "actor/abilities/abilities_data.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "physics/physics_body_data.hpp"
#include "tile_map/tile_map.hpp"
#include <cstddef>
#include "tile_map/tile_map_data.hpp"
#include <vector>
#include <cmath>

namespace
{
    AbilitiesData walkerAbilities()
    {
        AbilitiesData abilitiesData;
        abilitiesData.move = MoveAbilityData{};
        abilitiesData.gravity = GravityAbilityData{};
        return abilitiesData;
    }

    AbilitiesData jumperAbilities()
    {
        AbilitiesData abilitiesData = walkerAbilities();
        abilitiesData.jump = JumpAbilityData{};
        return abilitiesData;
    }

    float peakHeightOf(const std::vector<glm::vec2> &arc)
    {
        float highest = 0.0f;
        for (const glm::vec2 &offset : arc)
            highest = std::min(highest, offset.y);
        return -highest;
    }

    float reachOf(const std::vector<glm::vec2> &arc)
    {
        float furthest = 0.0f;
        for (const glm::vec2 &offset : arc)
            furthest = std::max(furthest, std::abs(offset.x));
        return furthest;
    }
}

TEST_CASE("An actor without a jump ability has no arc", "[JumpArc]")
{
    REQUIRE(simulateJumpArc(walkerAbilities()).offsets.empty());
}

TEST_CASE("An actor without gravity never comes back down", "[JumpArc]")
{
    AbilitiesData abilitiesData = jumperAbilities();
    abilitiesData.gravity.reset();

    REQUIRE(simulateJumpArc(abilitiesData).offsets.empty());
}

TEST_CASE("An arc leaves from where the actor stands", "[JumpArc]")
{
    std::vector<glm::vec2> arc = simulateJumpArc(jumperAbilities()).offsets;

    REQUIRE_FALSE(arc.empty());
    REQUIRE(arc.front() == glm::vec2(0.0f));
}

TEST_CASE("An arc rises and then returns to the height it left", "[JumpArc]")
{
    std::vector<glm::vec2> arc = simulateJumpArc(jumperAbilities()).offsets;

    REQUIRE(peakHeightOf(arc) > 0.0f);
    REQUIRE(arc.back().y >= 0.0f);
}

TEST_CASE("An arc only ever moves further from where it left", "[JumpArc]")
{
    std::vector<glm::vec2> arc = simulateJumpArc(jumperAbilities()).offsets;

    for (size_t index = 1; index < arc.size(); ++index)
        REQUIRE(arc[index].x >= arc[index - 1].x);
}

TEST_CASE("The default jump rises between 48 and 64 and crosses between 128 and 144", "[JumpArc]")
{
    std::vector<glm::vec2> arc = simulateJumpArc(jumperAbilities()).offsets;

    REQUIRE(peakHeightOf(arc) > 48.0f);
    REQUIRE(peakHeightOf(arc) < 64.0f);
    REQUIRE(reachOf(arc) > 128.0f);
    REQUIRE(reachOf(arc) < 144.0f);
}

TEST_CASE("A stronger jump reaches higher than a weaker one", "[JumpArc]")
{
    AbilitiesData weak = jumperAbilities();
    weak.jump->jumpSpeed = -150.0f;

    REQUIRE(
        peakHeightOf(simulateJumpArc(weak).offsets) <
        peakHeightOf(simulateJumpArc(jumperAbilities()).offsets));
}

TEST_CASE("A jump held longer reaches further than one cut short", "[JumpArc]")
{
    AbilitiesData brief = jumperAbilities();
    brief.jump->jumpDuration = 0.1f;

    REQUIRE(
        reachOf(simulateJumpArc(brief).offsets) <
        reachOf(simulateJumpArc(jumperAbilities()).offsets));
}

TEST_CASE("An actor that cannot move jumps straight up", "[JumpArc]")
{
    AbilitiesData abilitiesData = jumperAbilities();
    abilitiesData.move.reset();

    std::vector<glm::vec2> arc = simulateJumpArc(abilitiesData).offsets;

    REQUIRE_FALSE(arc.empty());
    REQUIRE(peakHeightOf(arc) > 0.0f);
    REQUIRE(reachOf(arc) == 0.0f);
}

TEST_CASE("An arc knows how long the jump was held for", "[JumpArc]")
{
    JumpArc arc = simulateJumpArc(jumperAbilities());

    REQUIRE(arc.holdDuration == jumperAbilities().jump->jumpDuration);
}

TEST_CASE("A shorter hold is recorded as one", "[JumpArc]")
{
    JumpArc arc = simulateJumpArc(jumperAbilities(), 0.5f);

    REQUIRE(arc.holdDuration == jumperAbilities().jump->jumpDuration * 0.5f);
    REQUIRE(reachOf(arc.offsets) < reachOf(simulateJumpArc(jumperAbilities()).offsets));
}

TEST_CASE("Every arc offered carries its own hold", "[JumpArc]")
{
    std::vector<JumpArc> arcs = simulateJumpArcs(jumperAbilities());

    REQUIRE(arcs.size() > 1);
    for (size_t index = 1; index < arcs.size(); ++index)
    {
        REQUIRE(arcs[index].holdDuration < arcs[index - 1].holdDuration);
        REQUIRE(reachOf(arcs[index].offsets) < reachOf(arcs[index - 1].offsets));
    }
}

TEST_CASE("A jump comes to rest on the surface, not beside it", "[JumpArc]")
{
    constexpr int TileSize = 16;
    std::vector<std::vector<int>> rows(12, std::vector<int>(30, 0));
    for (int x = 0; x < 30; ++x)
        rows[9][x] = 1;
    for (int x = 16; x < 30; ++x)
        rows[7][x] = 1;

    TileMapData tileMapData;
    tileMapData.indices = rows;
    tileMapData.tilePalette = "default";
    TileMap tileMap(tileMapData, theOnlyPalette(aPaletteWithASolidTile()));

    PhysicsBodyData physicsBodyData;
    physicsBodyData.colliderSize = glm::vec2(8.0f, 13.0f);
    physicsBodyData.colliderOffset = glm::vec2(4.0f, 3.0f);

    bool landedSomewhere = false;
    for (float takeOffX = 200.0f; takeOffX <= 250.0f; takeOffX += 1.0f)
    {
        JumpAttempt attempt = simulateJumpAgainst(
            tileMap,
            jumperAbilities(),
            physicsBodyData,
            glm::vec2(takeOffX, 9.0f * TileSize),
            1.0f,
            1.0f);
        if (!attempt.landed)
            continue;

        landedSomewhere = true;
        INFO("took off at " << takeOffX << ", came down at " << attempt.path.back().y);
        REQUIRE(std::fmod(attempt.path.back().y, static_cast<float>(TileSize)) == 0.0f);
    }

    REQUIRE(landedSomewhere);
}

#include "actor/abilities/abilities.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"
#include "timing/fixed_time_step.hpp"

TEST_CASE("An arc the builder simulates is the path the game's own steps take", "[JumpArc]")
{
    AbilitiesData abilitiesData = jumperAbilities();
    std::vector<glm::vec2> arc = simulateJumpArc(abilitiesData).offsets;
    REQUIRE(arc.size() > 2);

    Abilities abilities(abilitiesData);
    AbilityStates states;
    Observed observed;
    InputIntentions holding;
    holding.direction.x = 1.0f;
    holding.jumpRequested = true;
    holding.jumpHeld = true;
    observed.contacts.onGround = true;

    std::vector<glm::vec2> walked{glm::vec2(0.0f)};
    FixedTimeStep timestepper;
    timestepper.run(
        PhysicsStep * static_cast<float>(arc.size() - 1),
        [&](float dt)
        {
            walked.push_back(walked.back() + abilities.decide(dt, holding, observed, states) * dt);
            observed.contacts.onGround = false;
        });

    REQUIRE(walked.size() == arc.size());
    for (std::size_t at = 0; at < arc.size(); ++at)
    {
        REQUIRE(walked[at].x == Catch::Approx(arc[at].x));
        REQUIRE(walked[at].y == Catch::Approx(arc[at].y));
    }
}

TEST_CASE("An attempt that lands says how many steps it took", "[JumpArc]")
{
    TileMap tileMap = aTileMap({{{0, 5}, 1}, {{1, 5}, 1}, {{2, 5}, 1}, {{3, 5}, 1}});
    AbilitiesData abilitiesData = jumperAbilities();
    PhysicsBodyData body;

    JumpAttempt attempt =
        simulateJumpAgainst(tileMap, abilitiesData, body, tileMap.feetOnTile({1, 4}), 1.0f, 1.0f);

    REQUIRE(attempt.landed);
    REQUIRE_FALSE(attempt.capped);
    REQUIRE(attempt.steps == static_cast<int>(attempt.path.size()) - 1);
    REQUIRE(attempt.steps > 1);
}

TEST_CASE("An attempt that never lands is capped and says so", "[JumpArc]")
{
    TileMap tileMap = aTileMap({{{0, 9}, 1}}, 4, 10);
    AbilitiesData abilitiesData = jumperAbilities();
    abilitiesData.gravity.reset();
    PhysicsBodyData body;

    JumpAttempt attempt =
        simulateJumpAgainst(tileMap, abilitiesData, body, tileMap.feetOnTile({0, 8}), 1.0f, 1.0f);

    REQUIRE_FALSE(attempt.landed);
    REQUIRE(attempt.capped);
    REQUIRE(attempt.steps == 1000);
}

TEST_CASE("A report adds up what its attempts cost", "[JumpArc]")
{
    NavigationBuildReport report;
    JumpAttempt landed;
    landed.steps = 12;
    JumpAttempt capped;
    capped.steps = 1000;
    capped.capped = true;

    report.noting(landed);
    report.noting(capped);

    REQUIRE(report == NavigationBuildReport{1012, 1});
}
