#include <cfloat>
#include <optional>
#include <string>
#include <imgui.h>
#include "ui/levels_ui.hpp"
#include "ui/switching_level.hpp"
#include "ui/unsaved_colours.hpp"
#include "ui/saveable.hpp"
#include "game/levels.hpp"
#include "game/level.hpp"
#include "ui/editor_commands.hpp"

namespace
{
    std::optional<std::string> levelChooser(const char *label, const std::string &current)
    {
        std::optional<std::string> chosen;

        ImGui::TextUnformatted(label);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(("##" + std::string(label)).c_str(), levelName(current).c_str()))
        {
            for (const std::string &path : levelPathsIn(directoryOf(current)))
            {
                bool isCurrent = path == current;
                if (ImGui::Selectable(levelName(path).c_str(), isCurrent) && !isCurrent)
                    chosen = path;
            }
            ImGui::EndCombo();
        }

        return chosen;
    }
}

void LevelsUi::draw(
    Levels &levels,
    const Level &level,
    EditorCommands &commands,
    bool levelHasUnsavedChanges)
{
    std::optional<std::string> chosen = levelChooser("playing", level.getPath());

    bool switchPressed = false;
    bool cancelPressed = false;
    if (askedToSwitchTo && levelHasUnsavedChanges)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, UnsavedColour);
        ImGui::TextWrapped(
            "switching to %s discards unsaved changes to %s",
            levelName(*askedToSwitchTo).c_str(),
            levelName(level.getPath()).c_str());
        ImGui::PopStyleColor();
        switchPressed = ImGui::Button("switch");
        ImGui::SameLine();
        cancelPressed = ImGui::Button("cancel");
    }

    SwitchingLevel decided =
        switching(chosen, levelHasUnsavedChanges, askedToSwitchTo, switchPressed, cancelPressed);
    askedToSwitchTo = decided.waitingOn;
    if (decided.loadNow)
        commands.onLoadLevel(*decided.loadNow);

    ImGui::Separator();

    if (std::optional<std::string> first = levelChooser("first", levels.getFirst()))
        levels.setFirst(*first);
}
void LevelsUi::revert(Levels &levels)
{
    levels.setFirst(saveable.lastSeen("levels"));
}

void LevelsUi::save(Levels &levels)
{
    levels.save();
    saveable.saved("levels", levels.getFirst());
}

bool LevelsUi::unsavedSince(const Levels &levels)
{
    return saveable.unsavedSince("levels", levels.getFirst());
}

void LevelsUi::valuesReplaced()
{
    askedToSwitchTo.reset();
    saveable.valuesReplaced();
}
