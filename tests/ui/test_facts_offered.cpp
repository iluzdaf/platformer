#include <cstddef>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "conditions/asked.hpp"
#include "conditions/facts.hpp"
#include "conditions/when_data.hpp"
#include "helpers/headless_imgui.hpp"
#include "ui/data_inspector.hpp"
#include "ui/in_scope.hpp"
#include "ui/state_machine_field.hpp"
#include "ui/state_machine_shown.hpp"

namespace
{
    std::size_t drawnUnfolded(const std::function<void()> &draw)
    {
        HeadlessImGui gui;
        std::vector<ImGuiID> drawn;
        for (int pass = 0; pass < 8; ++pass)
        {
            gui.frame(
                [&]
                {
                    for (ImGuiID id : drawn)
                        ImGui::GetStateStorage()->SetInt(id, 1);
                    draw();
                });
            drawn = gui.everythingDrawn();
        }

        return drawn.size();
    }

    WhenData asking(const std::string &name, const Asked &asked)
    {
        WhenData when;
        when[name] = asked;
        return when;
    }

    std::size_t drawnForARuleAsking(const WhenData &when)
    {
        AnimatorData animations;
        animations.startClip = "idle";
        animations.clips["idle"] = FrameAnimationData{{0}, 0.5f};
        animations.clips["sleep"] = FrameAnimationData{{1}, 0.5f};
        animations.rules = {{"sleep", when}};

        return drawnUnfolded([&] { inspector::draw("animationData", animations); });
    }

    std::size_t drawnForATransitionAsking(const WhenData &when, const FactsData &declared)
    {
        BehaviorStateData sleeping;
        sleeping.name = "sleep";
        BehaviorStateData charging;
        charging.name = "charge";
        std::optional<StateMachineBehaviorData> machine =
            StateMachineBehaviorData{{sleeping, charging}, {{"sleep", "charge", when, 0.0f}}};
        MachineShown shown{MachineShown::What::Transition, 0};

        return drawnUnfolded(
            [&]
            {
                InScope declaring(declared);
                drawStateMachineEditor(machine, {}, shown);
            });
    }
}

TEST_CASE("An animation rule shows what it asks about its picture", "[FactsOffered]")
{
    REQUIRE(
        drawnForARuleAsking(asking("inState", std::string("sleep"))) >
        drawnForARuleAsking(WhenData{}));
    REQUIRE(drawnForARuleAsking(asking("finished", true)) > drawnForARuleAsking(WhenData{}));
}

TEST_CASE("A transition shows what it asks about the actor and what is declared", "[FactsOffered]")
{
    FactsData declared;
    declared["heard"] = false;

    REQUIRE(
        drawnForATransitionAsking(asking("heard", true), declared) >
        drawnForATransitionAsking(WhenData{}, declared));
    REQUIRE(
        drawnForATransitionAsking(asking("cornered", true), declared) >
        drawnForATransitionAsking(WhenData{}, declared));
}
