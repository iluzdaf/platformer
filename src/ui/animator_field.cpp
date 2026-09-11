#include <array>
#include <cstddef>
#include <set>
#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/marked_label.hpp"
#include "ui/animator_field.hpp"
#include "ui/saved_in_scope.hpp"
#include "ui/selection_in_scope.hpp"
#include "ui/data_inspector.hpp"
#include "ui/graph_shown.hpp"
#include "ui/graph_view.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/state_machine_shown.hpp"
#include "animations/animation_ladder_data.hpp"
#include "animations/animator_data.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"

namespace
{
    MachineShown rememberedSelection()
    {
        if (const MachineShown *asked = selectionInScope())
            return *asked;

        ImGuiStorage *storage = ImGui::GetStateStorage();
        int what = storage->GetInt(ImGui::GetID("shownWhat"), 0);
        int index = storage->GetInt(ImGui::GetID("shownIndex"), 0);
        return MachineShown{static_cast<MachineShown::What>(what), static_cast<std::size_t>(index)};
    }

    void remember(MachineShown shown)
    {
        if (selectionInScope())
            return;

        ImGuiStorage *storage = ImGui::GetStateStorage();
        storage->SetInt(ImGui::GetID("shownWhat"), static_cast<int>(shown.what));
        storage->SetInt(ImGui::GetID("shownIndex"), static_cast<int>(shown.index));
    }

    const std::string &clipNameShown(const GraphShown &graph, MachineShown shown)
    {
        static const std::string none;
        if (shown.what != MachineShown::What::State)
            return none;

        return graph.nodes[shown.index].name;
    }

    bool drawAddingAClip(AnimatorData &animations, MachineShown &shown)
    {
        if (ImGui::SmallButton("add clip"))
            ImGui::OpenPopup("##addClip");

        bool added = false;
        if (ImGui::BeginPopup("##addClip"))
        {
            static std::array<char, 64> asked{};
            ImGui::SetNextItemWidth(120.0f);
            ImGui::InputTextWithHint("##clipName", "name", asked.data(), asked.size());
            std::string wanted = asked.data();
            ImGui::SameLine();
            ImGui::BeginDisabled(wanted.empty() || animations.clips.contains(wanted));
            if (ImGui::Button("add"))
            {
                animations.clips.emplace(wanted, FrameAnimationData{});
                asked.fill(0);
                added = true;
                shown = MachineShown{};
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndDisabled();
            ImGui::EndPopup();
        }

        return added;
    }

    bool drawAddingARung(AnimatorData &animations, const GraphShown &graph, MachineShown &shown)
    {
        ImGui::SameLine();
        ImGui::BeginDisabled(animations.clips.empty());
        bool add = ImGui::SmallButton("add rung");
        ImGui::EndDisabled();
        if (!add)
            return false;

        const std::string &chosen = clipNameShown(graph, shown);
        AnimationTransitionData rung;
        rung.to = chosen.empty() || chosen == AnyNode ? std::string(IdleClip) : chosen;
        animations.ladder.transitions.push_back(rung);
        shown = showingTransition(animations.ladder.transitions.size() - 1);
        return true;
    }

    bool drawRemoving(AnimatorData &animations, const GraphShown &graph, MachineShown &shown)
    {
        ImGui::SameLine();
        bool removable =
            shown.what == MachineShown::What::Transition ||
            (shown.what == MachineShown::What::State && clipNameShown(graph, shown) != AnyNode);
        ImGui::BeginDisabled(!removable);
        bool remove = ImGui::SmallButton("remove");
        ImGui::EndDisabled();
        if (!remove)
            return false;

        if (shown.what == MachineShown::What::State)
            animations.clips.erase(clipNameShown(graph, shown));
        else
            animations.ladder.transitions.erase(
                animations.ladder.transitions.begin() + static_cast<std::ptrdiff_t>(shown.index));

        shown = MachineShown{};
        return true;
    }

    inspector::Edited drawShown(
        AnimatorData &animations,
        const GraphShown &graph,
        MachineShown shown)
    {
        ImGui::Separator();
        if (shown.what == MachineShown::What::Transition)
        {
            inspector::InField ladder("ladder");
            inspector::InField transitions("transitions");
            inspector::InField rung(std::to_string(shown.index));
            return inspector::drawFields(animations.ladder.transitions[shown.index]);
        }

        if (shown.what == MachineShown::What::State)
        {
            const std::string &name = clipNameShown(graph, shown);
            if (name == AnyNode)
            {
                ImGui::TextDisabled("rungs from here fire from whatever clip is playing");
                return {};
            }

            inspector::InField clips("clips");
            inspector::InField clip(name);
            return drawCustomField(name, animations.clips.at(name));
        }

        ImGui::TextDisabled("pick a clip or a rung");
        return {};
    }
}

MachineShown drawAnimatorGraph(
    const AnimatorData &animations,
    const std::set<std::string> &litClips,
    MachineShown selected)
{
    return drawGraph("##animatorGraph", graphOf(animations), litClips, selected);
}

inspector::Edited drawCustomField(std::string_view name, AnimatorData &value)
{
    if (!inspector::drawFold(name))
        return {};

    inspector::Edited edited;
    MachineShown shown = rememberedSelection();
    GraphShown graph = graphOf(value);
    shown = stillAmong(shown, graph);

    bool changed = drawAddingAClip(value, shown);
    changed = drawAddingARung(value, graph, shown) || changed;
    changed = drawRemoving(value, graph, shown) || changed;
    if (changed)
    {
        edited |= inspector::Edited{true, true};
        graph = graphOf(value);
        shown = stillAmong(shown, graph);
    }

    shown = drawGraph("##animatorGraph", graph, {}, shown);
    edited |= drawShown(value, graph, shown);
    remember(shown);

    ImGui::TreePop();
    return edited;
}
