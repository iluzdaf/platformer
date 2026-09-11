#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <vector>
#include <map>
#include <optional>
#include <tuple>
#include <string_view>
#include "helpers/headless_imgui.hpp"
#include <variant>
#include <glaze/glaze.hpp>
#include "game/game_data.hpp"
#include "game/levels_data.hpp"
#include "cameras/camera2d_data.hpp"
#include "player/player_data.hpp"
#include "pickups/pickup_data.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "ui/data_inspector.hpp"
#include "ui/saveable.hpp"
#include "ui/state_machine_field.hpp"
#include "ui/state_machine_shown.hpp"
#include "ui/graph_shown.hpp"
#include "npc/npc_data.hpp"
#include "actor/actor_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "ui/inspector_edited.hpp"
#include <string>
#include "serialization/only_what_differs.hpp"
#include "ui/saved_in_scope.hpp"

namespace saved
{
    struct WheelsData
    {
        int count = 4;
        float grip = 1.0f;
    };

    struct CarData
    {
        std::string name;
        WheelsData wheels;
        std::optional<WheelsData> spare;
    };

    std::string drawnAt;

    inspector::Edited drawCustomField(std::string_view, WheelsData &)
    {
        drawnAt = inspector::pathHere();
        return {};
    }
}

TEST_CASE("With nothing saved in scope, nothing has changed", "[SavedInScope]")
{
    saved::CarData car;
    car.name = "rusty";

    REQUIRE_FALSE(inspector::changedHere(car));
}

TEST_CASE("A field that matches what was saved has not changed", "[SavedInScope]")
{
    saved::CarData car;
    car.name = "rusty";
    SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));

    REQUIRE_FALSE(inspector::changedHere(car));

    inspector::InField name("name");
    REQUIRE_FALSE(inspector::changedHere(car.name));
}

TEST_CASE("A field edited away from what was saved has changed", "[SavedInScope]")
{
    saved::CarData car;
    car.name = "rusty";
    SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));

    car.name = "shiny";

    REQUIRE(inspector::changedHere(car));

    inspector::InField name("name");
    REQUIRE(inspector::changedHere(car.name));
}

TEST_CASE("A field under a field is found by the path it is drawn at", "[SavedInScope]")
{
    saved::CarData car;
    SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));

    car.wheels.grip = 0.5f;

    inspector::InField wheels("wheels");
    REQUIRE(inspector::changedHere(car.wheels));

    inspector::InField count("count");
    REQUIRE_FALSE(inspector::changedHere(car.wheels.count));
}

TEST_CASE(
    "A field the saved copy said nothing about is its default until it is edited",
    "[SavedInScope]")
{
    saved::CarData car;
    SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));

    inspector::InField wheels("wheels");
    inspector::InField grip("grip");
    REQUIRE_FALSE(inspector::changedHere(car.wheels.grip));

    car.wheels.grip = 0.5f;
    REQUIRE(inspector::changedHere(car.wheels.grip));
}

TEST_CASE("A field of something the saved copy never had has changed", "[SavedInScope]")
{
    std::map<std::string, saved::CarData> cars{{"rusty", saved::CarData{}}};
    SavedInScope was(asItWasSaved<std::map<std::string, saved::CarData>>(onlyWhatDiffers(cars)));

    cars["shiny"] = saved::CarData{};

    inspector::InField added("shiny");
    REQUIRE(inspector::changedHere(cars.at("shiny")));

    inspector::InField name("name");
    REQUIRE(inspector::changedHere(cars.at("shiny").name));
}

TEST_CASE("What was saved is out of scope once the scope is", "[SavedInScope]")
{
    saved::CarData car;
    {
        SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));
        car.name = "shiny";
        REQUIRE(inspector::changedHere(car));
    }

    REQUIRE_FALSE(inspector::changedHere(car));
}

TEST_CASE("What an optional holds is drawn where the optional is", "[SavedInScope]")
{
    HeadlessImGui gui;
    saved::CarData car;
    car.spare = saved::WheelsData{};
    SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));

    saved::drawnAt.clear();
    gui.frame([&] { std::ignore = inspector::draw("spare", car.spare); });

    REQUIRE(saved::drawnAt == "spare");
}

TEST_CASE("A field inside an optional that was saved has not changed", "[SavedInScope]")
{
    saved::CarData car;
    car.spare = saved::WheelsData{};
    SavedInScope was(asItWasSaved<saved::CarData>(onlyWhatDiffers(car)));

    inspector::InField spare("spare");
    REQUIRE_FALSE(inspector::changedHere(car.spare));
    REQUIRE_FALSE(inspector::changedHere(*car.spare));

    car.spare->count = 3;

    REQUIRE(inspector::changedHere(car.spare));
    REQUIRE(inspector::changedHere(*car.spare));
}

namespace
{
    template <class T> void eachFieldOf(T &value);

    template <class T> void eachFieldUnder(std::string_view name, T &value)
    {
        inspector::InField here(name);
        std::ignore = inspector::changedHere(value);
        eachFieldOf(value);
    }

    template <class T> void eachFieldOf(T &value)
    {
        if constexpr (inspector::IsOptional<T>::value)
        {
            if (value)
                eachFieldOf(*value);
        }
        else if constexpr (inspector::IsVector<T>::value)
        {
            for (std::size_t at = 0; at < value.size(); ++at)
                eachFieldUnder(std::to_string(at), value[at]);
        }
        else if constexpr (inspector::IsMap<T>::value)
        {
            for (auto &[key, held] : value)
                eachFieldUnder(inspector::keyLabel(key), held);
        }
        else if constexpr (inspector::IsVariant<T>::value && glz::tagged<T>)
        {
            std::visit(
                [](auto &held)
                {
                    inspector::forEachNamedField(
                        held,
                        [](std::string_view fieldName, auto &field)
                        { eachFieldUnder(fieldName, field); });
                },
                value);
        }
        else if constexpr (glz::reflectable<T>)
        {
            inspector::forEachNamedField(
                value,
                [](std::string_view fieldName, auto &field) { eachFieldUnder(fieldName, field); });
        }
    }
}

TEST_CASE("Every field of the shipped game data is found where it was saved", "[SavedInScope]")
{
    GameData gameData = loadGameData();
    std::vector<std::string> lost;

    auto lookFrom = [&](const std::string &savedJson, auto &value)
    {
        SavedInScope was(savedJson);
        inspector::Watching watching;
        std::ignore = inspector::changedHere(value);
        eachFieldOf(value);
        for (const std::string &path : watching.saidChanged())
            lost.push_back(path);
    };

    lookFrom(asItWasSaved<GameSettingsData>(asJson(gameData.settings)), gameData.settings);
    lookFrom(asItWasSaved<Camera2DData>(asJson(gameData.cameraData)), gameData.cameraData);
    lookFrom(asItWasSaved<PlayerData>(asJson(gameData.playerData)), gameData.playerData);
    lookFrom(
        asItWasSaved<std::map<std::string, NpcData>>(asJson(gameData.npcData)), gameData.npcData);
    lookFrom(
        asItWasSaved<std::map<std::string, PickupData>>(asJson(gameData.pickupData)),
        gameData.pickupData);
    lookFrom(asItWasSaved<TilePalettes>(asJson(gameData.tilePalettes)), gameData.tilePalettes);
    lookFrom(asItWasSaved<LevelsData>(asJson(gameData.levels)), gameData.levels);

    INFO("fields the saved copy was not asked about where they are: " << lost.size());
    for (const std::string &path : lost)
        UNSCOPED_INFO(path);

    REQUIRE(lost.empty());
}

TEST_CASE("The machine editor looks for a state where the npc keeps it", "[SavedInScope]")
{
    HeadlessImGui gui;
    NpcData npc = loadGameData().npcData.at("rat");
    REQUIRE(npc.stateMachineBehaviorData);
    REQUIRE_FALSE(npc.stateMachineBehaviorData->states.empty());
    REQUIRE_FALSE(npc.stateMachineBehaviorData->transitions.empty());

    auto lookingAt = [&](MachineShown shown)
    {
        std::vector<std::string> changed;
        gui.frame(
            [&]
            {
                SavedInScope was(asItWasSaved<NpcData>(asJson(npc)));
                inspector::InField machine("stateMachineBehaviorData");
                inspector::Watching watching;
                MachineShown showing = shown;
                std::ignore = drawStateMachineEditor(npc.stateMachineBehaviorData, {}, showing);
                changed = watching.saidChanged();
            });

        return changed;
    };

    REQUIRE(lookingAt(showingState(0)).empty());
    REQUIRE(lookingAt(showingTransition(0)).empty());
}

TEST_CASE("The animator editor looks for a clip where the actor keeps it", "[SavedInScope]")
{
    HeadlessImGui gui;
    ActorData actor = loadGameData().playerData.actorData;
    GraphShown graph = graphOf(actor.animationData);

    std::size_t clip = graph.nodes.size();
    for (std::size_t at = 0; at < graph.nodes.size(); ++at)
        if (graph.nodes[at].name != AnyNode)
        {
            clip = at;
            break;
        }

    REQUIRE(clip < graph.nodes.size());
    const std::string &clipName = graph.nodes[clip].name;

    auto lookingAt = [&](MachineShown shown)
    {
        std::vector<std::string> changed;
        std::size_t looked = 0;
        auto remembering = [&]
        {
            ImGui::TreeNodeSetOpen(ImGui::GetID("animationData"), true);
            ImGui::PushOverrideID(ImGui::GetID("animationData"));
            ImGui::GetStateStorage()->SetInt(
                ImGui::GetID("shownWhat"), static_cast<int>(shown.what));
            ImGui::GetStateStorage()->SetInt(
                ImGui::GetID("shownIndex"), static_cast<int>(shown.index));
            ImGui::TreeNodeSetOpen(ImGui::GetID(clipName.c_str()), true);
            ImGui::PopID();
        };

        gui.frame(remembering);
        gui.frame(
            [&]
            {
                remembering();
                SavedInScope was(asItWasSaved<ActorData>(asJson(actor)));
                inspector::Watching watching;
                std::ignore = inspector::draw("animationData", actor.animationData);
                changed = watching.saidChanged();
                looked = watching.looked().size();
            });

        INFO(
            "looked at " << looked << ", wrong place for "
                         << (changed.empty() ? "" : changed.front()));
        REQUIRE(looked > 1);
        return changed;
    };

    REQUIRE(lookingAt(showingState(clip)).empty());
    REQUIRE(lookingAt(showingTransition(0)).empty());
}
