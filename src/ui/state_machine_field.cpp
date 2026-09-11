#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <imgui.h>
#include "ui/state_machine_field.hpp"
#include "ui/saved_in_scope.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/state_machine_graph.hpp"
#include "ui/state_machine_shown.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"

namespace
{
    std::string aStateNameNobodyHasTaken(const StateMachineBehaviorData &machine)
    {
        std::string name = "new";
        for (int suffix = 2; indexOfState(machine, name); ++suffix)
            name = "new " + std::to_string(suffix);

        return name;
    }

    const std::string &firstStateNameFor(
        const StateMachineBehaviorData &machine,
        MachineShown shown)
    {
        if (shown.what == MachineShown::What::State)
            return machine.states[shown.index].name;

        return machine.states.front().name;
    }

    bool drawAdding(StateMachineBehaviorData &machine, MachineShown &shown)
    {
        if (ImGui::SmallButton("add state"))
        {
            BehaviorStateData state;
            state.name = aStateNameNobodyHasTaken(machine);
            machine.states.push_back(state);
            shown = showingState(machine.states.size() - 1);
            return true;
        }

        ImGui::SameLine();
        ImGui::BeginDisabled(machine.states.empty());
        bool addTransition = ImGui::SmallButton("add transition");
        ImGui::EndDisabled();
        if (addTransition)
        {
            BehaviorTransitionData transition;
            transition.from = firstStateNameFor(machine, shown);
            transition.to = transition.from;
            machine.transitions.push_back(transition);
            shown = showingTransition(machine.transitions.size() - 1);
            return true;
        }

        return false;
    }

    bool drawRemoving(StateMachineBehaviorData &machine, MachineShown &shown)
    {
        ImGui::SameLine();
        ImGui::BeginDisabled(shown.what == MachineShown::What::Nothing);
        bool remove = ImGui::SmallButton("remove");
        ImGui::EndDisabled();
        if (!remove)
            return false;

        auto at = static_cast<std::ptrdiff_t>(shown.index);
        if (shown.what == MachineShown::What::State)
            machine.states.erase(machine.states.begin() + at);
        else
            machine.transitions.erase(machine.transitions.begin() + at);

        shown = MachineShown{};
        return true;
    }

    inspector::Edited drawShown(StateMachineBehaviorData &machine, MachineShown shown)
    {
        ImGui::Separator();
        if (shown.what == MachineShown::What::State)
        {
            inspector::InField states("states");
            inspector::InField state(std::to_string(shown.index));
            return inspector::drawFields(machine.states[shown.index]);
        }

        if (shown.what == MachineShown::What::Transition)
        {
            inspector::InField transitions("transitions");
            inspector::InField transition(std::to_string(shown.index));
            return inspector::drawFields(machine.transitions[shown.index]);
        }

        ImGui::TextDisabled("pick a state or a transition");
        return {};
    }
}

inspector::Edited drawStateMachineEditor(
    std::optional<StateMachineBehaviorData> &machine,
    const std::set<std::string> &litStates,
    MachineShown &shown)
{
    bool present = machine.has_value();
    bool toggled = ImGui::Checkbox("stateMachineBehaviorData", &present);
    inspector::Edited edited = inspector::justEdited(toggled);
    if (toggled)
    {
        machine = present ? std::optional(StateMachineBehaviorData{}) : std::nullopt;
        shown = MachineShown{};
    }

    if (!machine)
        return edited;

    ImGui::PushID("machine");
    bool changed = drawAdding(*machine, shown);
    changed = drawRemoving(*machine, shown) || changed;
    if (changed)
        edited |= inspector::Edited{true, true};

    shown = stillAmong(shown, *machine);
    shown = drawStateMachineGraph(*machine, litStates, shown);
    edited |= drawShown(*machine, shown);
    ImGui::PopID();

    return edited;
}
