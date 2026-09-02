#include <cfloat>
#include <optional>
#include <string>
#include <imgui.h>
#include "ui/levels_ui.hpp"
#include "ui/save_controls.hpp"
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

SwitchingLevel switching(
    const std::optional<std::string> &chosen,
    bool levelHasUnsavedChanges,
    const std::optional<std::string> &waitingOn,
    bool switchPressed,
    bool cancelPressed)
{
    std::optional<std::string> held = levelHasUnsavedChanges ? waitingOn : std::nullopt;

    if (chosen && !levelHasUnsavedChanges)
        return {chosen, std::nullopt};

    if (chosen)
        return {std::nullopt, chosen};

    if (held && switchPressed)
        return {held, std::nullopt};

    if (held && cancelPressed)
        return {std::nullopt, std::nullopt};

    return {std::nullopt, held};
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

    drawSaveControls(
        saveable,
        "levels",
        levels.getFirst(),
        [&levels] { levels.save(); },
        [&levels](const std::string &lastSeen) { levels.setFirst(lastSeen); });

    ImGui::Separator();

    if (std::optional<std::string> first = levelChooser("first", levels.getFirst()))
        levels.setFirst(*first);
}

bool LevelsUi::hasUnsavedChanges(const Levels &levels) const
{
    return saveable.unsaved("levels", levels.getFirst());
}

void LevelsUi::valuesReplaced()
{
    askedToSwitchTo.reset();
    saveable.valuesReplaced();
}
