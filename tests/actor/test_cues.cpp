#include <catch2/catch_test_macros.hpp>
#include "input/input_intentions.hpp"
#include "input/intention_source.hpp"
#include "actor/appearance.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "game/level.hpp"
#include "helpers/actors.hpp"
#include "helpers/floor_level.hpp"
#include "helpers/palettes.hpp"
#include "helpers/player_fixtures.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "timing/fixed_time_step.hpp"

TEST_CASE("An actor says the cues of the clip it is playing", "[Actor][Cues]")
{
    PlayerData playerData = playerDataWithEveryAbility();
    playerData.actorData.animationData->clips["idle"] =
        FrameAnimationData{{0, 1}, 0.05f, {{0, "onFootstep"}, {1, "onFootstep"}}};
    Player player(playerData, noIntentions());
    std::vector<std::string> heard;
    player.onCue.connect([&](const std::string &cue) { heard.push_back(cue); });

    Level level(
        aFloorLevelPlacing({}), theOnlyPalette(aPaletteWithASolidTile()), playerData, {}, {});
    FixedTimeStep timestepper;
    runFor(player, level, 0.5f, timestepper);

    REQUIRE(heard.size() >= 2);
    REQUIRE(std::ranges::all_of(heard, [](const std::string &cue) { return cue == "onFootstep"; }));
}

TEST_CASE("An actor whose clips have no cues says nothing", "[Actor][Cues]")
{
    PlayerData playerData = playerDataWithEveryAbility();
    Player player(playerData, noIntentions());
    int heard = 0;
    player.onCue.connect([&](const std::string &) { ++heard; });

    Level level(
        aFloorLevelPlacing({}), theOnlyPalette(aPaletteWithASolidTile()), playerData, {}, {});
    FixedTimeStep timestepper;
    runFor(player, level, 0.5f, timestepper);

    REQUIRE(heard == 0);
}

namespace
{
    PlayerData aPlayerWithAnAttackClip()
    {
        PlayerData playerData = playerDataWithEveryAbility();
        playerData.actorData.animationData->clips["attack"] =
            FrameAnimationData{{12, 13, 14}, 0.1f};
        playerData.actorData.animationData->clips.at("attack").loops = false;
        return playerData;
    }
}

namespace
{
    class PressingAttackOnce : public IntentionSource
    {
    public:
        void arm()
        {
            armed = true;
        }

        InputIntentions getIntentions() const override
        {
            InputIntentions intentions;
            if (armed && !pressed)
            {
                intentions.attack = std::string(SwingAttack);
                pressed = true;
            }
            return intentions;
        }

    private:
        bool armed = false;
        mutable bool pressed = false;
    };
}

TEST_CASE(
    "A swing timed like its clip strikes one tick behind the frame that shows the blade",
    "[Actor][Cues]")
{
    PlayerData playerData = aPlayerWithAnAttackClip();
    PressingAttackOnce once;
    Player player(playerData, once);
    Level level(
        aFloorLevelPlacing({}), theOnlyPalette(aPaletteWithASolidTile()), playerData, {}, {});
    FixedTimeStep timestepper;
    runFor(player, level, 0.3f, timestepper);
    once.arm();

    int shownLastTick = player.appearance().currentFrame;
    int ticksStriking = 0;
    bool rested = false;
    for (int step = 0; step < 60; ++step)
    {
        player.beginFrame();
        player.fixedUpdate(0.01f, level);

        INFO(
            "step " << step << ": shown last tick " << shownLastTick << ", now "
                    << player.appearance().currentFrame);
        if (player.abilityStates().swing.swinging())
            REQUIRE(player.abilityStates().swing.striking() == (shownLastTick == 13));
        ticksStriking += player.abilityStates().swing.striking();
        if (step > 5 && !player.abilityStates().swing.swinging())
            rested = true;
        shownLastTick = player.appearance().currentFrame;
    }

    REQUIRE(ticksStriking == 10);
    REQUIRE(rested);
}

TEST_CASE("A swing strikes without an animator to show it", "[Actor][Cues]")
{
    PlayerData playerData = playerDataWithEveryAbility();
    playerData.actorData.animationData.reset();
    PressingAttackOnce once;
    Player player(playerData, once);
    Level level(
        aFloorLevelPlacing({}), theOnlyPalette(aPaletteWithASolidTile()), playerData, {}, {});
    FixedTimeStep timestepper;
    runFor(player, level, 0.3f, timestepper);
    once.arm();

    int ticksStriking = 0;
    bool rested = false;
    for (int step = 0; step < 60; ++step)
    {
        player.beginFrame();
        player.fixedUpdate(0.01f, level);
        ticksStriking += player.abilityStates().swing.striking();
        if (step > 5 && !player.abilityStates().swing.swinging())
            rested = true;
    }

    REQUIRE(ticksStriking == 10);
    REQUIRE(rested);
}
