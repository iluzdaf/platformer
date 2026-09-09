#include <catch2/catch_test_macros.hpp>
#include "input/input_intentions.hpp"
#include "input/intention_source.hpp"
#include "actor/actor_state.hpp"
#include "actor/decided.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include <catch2/matchers/catch_matchers_string.hpp>
#include <algorithm>
#include <string>
#include <vector>
#include "actor/actor_animation_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "game/level.hpp"
#include "helpers/actors.hpp"
#include "helpers/levels.hpp"
#include "helpers/palettes.hpp"
#include "helpers/player_fixtures.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "timing/fixed_time_step.hpp"

TEST_CASE("An actor says the cues of the clip it is playing", "[Actor][Cues]")
{
    PlayerData playerData = playerDataWithEveryAbility();
    playerData.actorData.animationData.idle =
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
        playerData.actorData.animationData.attack = FrameAnimationData{
            {12, 13, 14}, 0.1f, {{1, std::string(StrikeCue)}, {2, std::string(RecoverCue)}}};
        playerData.actorData.animationData.attack->loops = false;
        return playerData;
    }
}

TEST_CASE("A swing is refused without a strike cue", "[Actor][Cues]")
{
    PlayerData playerData = aPlayerWithAnAttackClip();
    playerData.actorData.animationData.attack->cues.clear();

    REQUIRE_THROWS_WITH(
        Player(playerData, noIntentions()), Catch::Matchers::ContainsSubstring("onStrike"));
}

TEST_CASE("A swing is refused when its clip loops", "[Actor][Cues]")
{
    PlayerData playerData = aPlayerWithAnAttackClip();
    playerData.actorData.animationData.attack->loops = true;

    REQUIRE_THROWS_WITH(
        Player(playerData, noIntentions()), Catch::Matchers::ContainsSubstring("plays once"));
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

TEST_CASE("A swing strikes one tick behind the frame that shows the blade", "[Actor][Cues]")
{
    PlayerData playerData = aPlayerWithAnAttackClip();
    PressingAttackOnce once;
    Player player(playerData, once);
    Level level(
        aFloorLevelPlacing({}), theOnlyPalette(aPaletteWithASolidTile()), playerData, {}, {});
    FixedTimeStep timestepper;
    runFor(player, level, 0.3f, timestepper);
    once.arm();

    int shownLastTick = player.state().currentFrame;
    int ticksStriking = 0;
    bool rested = false;
    for (int step = 0; step < 60; ++step)
    {
        player.beginFrame();
        player.fixedUpdate(0.01f, level);

        INFO(
            "step " << step << ": shown last tick " << shownLastTick << ", now "
                    << player.state().currentFrame);
        if (player.decided().swing.swinging())
            REQUIRE(player.decided().swing.striking() == (shownLastTick == 13));
        ticksStriking += player.decided().swing.striking();
        if (step > 5 && !player.decided().swing.swinging())
            rested = true;
        shownLastTick = player.state().currentFrame;
    }

    REQUIRE(ticksStriking == 10);
    REQUIRE(rested);
}
