#include <string>
#include <variant>
#include <catch2/catch_test_macros.hpp>
#include <map>
#include "cameras/camera2d_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "npc/npc_data.hpp"
#include "ui/saveable.hpp"

TEST_CASE("Saveable has nothing to save until a value changes", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    saveable.seen("camera", asJson(data));

    REQUIRE_FALSE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("Saveable notices a changed value", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    saveable.seen("camera", asJson(data));
    data.zoom += 1.0f;

    REQUIRE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("Saveable keeps its first baseline however often it sees a value", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    saveable.seen("camera", asJson(data));
    data.zoom += 1.0f;
    saveable.seen("camera", asJson(data));

    REQUIRE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("Saveable moves its baseline when a value is saved", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    saveable.seen("camera", asJson(data));
    data.zoom += 1.0f;
    saveable.saved("camera", asJson(data));

    REQUIRE_FALSE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("A clean value follows the disk on a reload", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;
    saveable.seen("camera", asJson(data));

    Camera2DData onDisk;
    onDisk.zoom += 1.0f;
    reload(saveable, "camera", data, onDisk);

    REQUIRE(data.zoom == onDisk.zoom);
    REQUIRE_FALSE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("An unsaved value is kept through a reload and stays unsaved", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;
    saveable.seen("camera", asJson(data));
    data.zoom += 1.0f;
    float edited = data.zoom;

    Camera2DData onDisk;
    onDisk.zoom += 2.0f;
    reload(saveable, "camera", data, onDisk);

    REQUIRE(data.zoom == edited);
    REQUIRE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("A value never looked at follows the disk on a reload", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;
    data.zoom += 1.0f;

    Camera2DData onDisk;
    reload(saveable, "camera", data, onDisk);

    REQUIRE(data.zoom == onDisk.zoom);
    REQUIRE_FALSE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("Reverting after a reload goes to what is on disk now", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;
    saveable.seen("camera", asJson(data));
    data.zoom += 1.0f;

    Camera2DData onDisk;
    onDisk.zoom += 2.0f;
    reload(saveable, "camera", data, onDisk);
    revertTo(saveable, "camera", data);

    REQUIRE(data.zoom == onDisk.zoom);
    REQUIRE_FALSE(saveable.unsaved("camera", asJson(data)));
}

TEST_CASE("Saveable remembers the baseline a revert would go back to", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;
    std::string wasOnDisk = asJson(data);

    saveable.seen("camera", wasOnDisk);
    data.zoom += 1.0f;

    REQUIRE(saveable.lastSeen("camera") == wasOnDisk);
}

TEST_CASE("Saveable has nothing to say about a name it has never seen", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    REQUIRE_FALSE(saveable.unsaved("camera", asJson(data)));
    REQUIRE(saveable.lastSeen("camera").empty());
}

TEST_CASE("The first look is what everything after is compared against", "[Saveable]")
{
    Saveable saveable;

    REQUIRE_FALSE(saveable.unsavedSince("camera", "one"));
    REQUIRE(saveable.unsavedSince("camera", "two"));
    REQUIRE(saveable.unsavedSince("camera", "three"));
}

TEST_CASE("Saving moves what everything after is compared against", "[Saveable]")
{
    Saveable saveable;

    REQUIRE_FALSE(saveable.unsavedSince("camera", "one"));
    saveable.saved("camera", "two");

    REQUIRE_FALSE(saveable.unsavedSince("camera", "two"));
    REQUIRE(saveable.unsavedSince("camera", "one"));
}

namespace
{
    std::map<std::string, NpcData> aRatThat(const BehaviorDoes &does)
    {
        BehaviorStateData state;
        state.name = "one";
        state.does = does;
        NpcData rat;
        rat.stateMachineBehaviorData = StateMachineBehaviorData{{state}, {}};
        return {{"rat", rat}};
    }
}

TEST_CASE("A state that says its kind can be put back", "[Saveable]")
{
    Saveable saveable;
    ChaseBehaviorData chase;
    chase.standoff = 2.0f;
    std::map<std::string, NpcData> npcs = aRatThat(chase);
    std::map<std::string, NpcData> asSaved = npcs;
    saveable.saved("npcs", asJson(npcs));

    BehaviorDoes &does = npcs.at("rat").stateMachineBehaviorData->states.front().does;
    std::get<ChaseBehaviorData>(does).standoff = 4.0f;
    REQUIRE(saveable.unsaved("npcs", asJson(npcs)));

    revertTo(saveable, "npcs", npcs);

    const BehaviorDoes &back = npcs.at("rat").stateMachineBehaviorData->states.front().does;
    REQUIRE(std::holds_alternative<ChaseBehaviorData>(back));
    REQUIRE(std::get<ChaseBehaviorData>(back).standoff == 2.0f);
    REQUIRE(asJson(npcs) == asJson(asSaved));
}

TEST_CASE("Choosing another kind counts as unsaved", "[Saveable]")
{
    Saveable saveable;
    std::map<std::string, NpcData> npcs = aRatThat(ScriptedBehaviorData{"patrol"});
    saveable.saved("npcs", asJson(npcs));

    npcs.at("rat").stateMachineBehaviorData->states.front().does = ChaseBehaviorData{};

    REQUIRE(saveable.unsaved("npcs", asJson(npcs)));
}
