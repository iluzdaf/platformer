#include <string_view>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/melee_ability_data.hpp"
#include "actor/abilities/melee_ability_state.hpp"
#include "actor/actor_animation_state.hpp"
#include "actor/actor_state.hpp"
#include "actor/decided.hpp"
#include "actor/health.hpp"
#include "actor/health_data.hpp"
#include "actor/hit.hpp"
#include "animations/frame_animation_data.hpp"
#include "game/level.hpp"
#include "helpers/actors.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/palettes.hpp"
#include "helpers/player_fixtures.hpp"
#include "helpers/tiles.hpp"
#include "input/input_intentions.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/striking_npcs.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "timing/fixed_time_step.hpp"

namespace
{
    constexpr glm::ivec2 PlayerTile{4, FloorLevelStanding};

    PlayerData aSwordsman()
    {
        PlayerData playerData = playerDataWithEveryAbility();
        MeleeAbilityData swing;
        swing.windup = 0.05f;
        swing.active = 0.2f;
        swing.recovery = 0.05f;
        playerData.actorData.motionData.meleeAbilityData = swing;
        return playerData;
    }

    NpcData aVillagerWith(int points)
    {
        NpcData rat = setupNpcData();
        rat.actorData.healthData = HealthData{points, 0.0f};
        rat.actorData.animationData.dead = FrameAnimationData({7}, 1.0f);
        return rat;
    }

    struct Duel
    {
        Duel(int ratPoints, glm::ivec2 ratTile)
            : playerData(aSwordsman()), level(
                                            aFloorLevelPlacing({spawnAt("rat", ratTile)}),
                                            theOnlyPalette(aPaletteWithASolidTile()),
                                            playerData,
                                            {{"rat", aVillagerWith(ratPoints)}},
                                            {}),
              player(playerData, input)
        {
            player.standAt(feetOf(PlayerTile));
        }

        Npc &rat()
        {
            return *level.getNpcs().front();
        }

        void swingFor(float seconds)
        {
            InputIntentions attack;
            attack.attackRequested = true;
            input.set(attack);
            runFor(player, level, seconds, timestepper);
            input.set(InputIntentions{});
        }

        ScriptedIntentions input;
        PlayerData playerData;
        Level level;
        Player player;
        FixedTimeStep timestepper;
    };
}

TEST_CASE("A swing in front of the player costs the npc the swing's damage", "[StrikingNpcs]")
{
    Duel duel(3, PlayerTile + glm::ivec2(1, 0));
    duel.swingFor(0.1f);
    REQUIRE(duel.player.decided().melee.striking());

    strikeNpcs(duel.player, duel.level.getNpcs());

    REQUIRE(duel.rat().health().points() == 2);
    REQUIRE(duel.rat().health().lastHit()->direction == glm::vec2(1.0f, 0.0f));
}

TEST_CASE("A swing lands once, however long the npc stays in reach", "[StrikingNpcs]")
{
    Duel duel(3, PlayerTile + glm::ivec2(1, 0));
    duel.swingFor(0.1f);

    strikeNpcs(duel.player, duel.level.getNpcs());
    duel.swingFor(0.05f);
    REQUIRE(duel.player.decided().melee.striking());
    strikeNpcs(duel.player, duel.level.getNpcs());

    REQUIRE(duel.rat().health().points() == 2);
}

TEST_CASE("A swing behind the player misses", "[StrikingNpcs]")
{
    Duel duel(3, PlayerTile - glm::ivec2(1, 0));
    duel.swingFor(0.1f);

    strikeNpcs(duel.player, duel.level.getNpcs());

    REQUIRE(duel.rat().health().points() == 3);
}

TEST_CASE("Between swings, nothing lands", "[StrikingNpcs]")
{
    Duel duel(3, PlayerTile + glm::ivec2(1, 0));
    runFor(duel.player, duel.level, 0.1f, duel.timestepper);
    REQUIRE_FALSE(duel.player.swing());

    strikeNpcs(duel.player, duel.level.getNpcs());

    REQUIRE(duel.rat().health().points() == 3);
}

TEST_CASE("A swing reaches out from the collider on the side it faces", "[StrikingNpcs]")
{
    Duel duel(3, PlayerTile + glm::ivec2(1, 0));
    duel.swingFor(0.1f);
    AABB collider = duel.player.body().aabb();

    AABB reach = duel.player.swing().value();

    REQUIRE(reach.left() == collider.right());
    REQUIRE(reach.size == duel.playerData.actorData.motionData.meleeAbilityData->reach);
    REQUIRE(reach.center().y == collider.center().y);
}

TEST_CASE("A corpse stops deciding, shows it, and takes no more hits", "[StrikingNpcs]")
{
    Duel duel(1, PlayerTile + glm::ivec2(1, 0));
    duel.swingFor(0.1f);
    strikeNpcs(duel.player, duel.level.getNpcs());
    REQUIRE_FALSE(duel.rat().alive());

    duel.rat().beginFrame();
    duel.rat().fixedUpdate(0.01f, duel.level, duel.player.feet());

    REQUIRE(duel.rat().stateName() == std::string_view{});
    REQUIRE(duel.rat().state().currentAnimationState == ActorAnimationState::Dead);
    REQUIRE_FALSE(duel.player.strike(duel.rat()));
}
