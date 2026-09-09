#include <catch2/catch_test_macros.hpp>
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
