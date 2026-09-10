#include <cfloat>
#include <optional>
#include <string>
#include <imgui.h>
#include "ui/levels_ui.hpp"
#include "ui/data_inspector.hpp"
#include "game/level_data_file.hpp"
#include "ui/file_chooser.hpp"
#include "ui/switching_level.hpp"
#include "ui/unsaved_colours.hpp"
#include "ui/saveable.hpp"
#include "game/levels.hpp"
#include "game/levels_data.hpp"
#include "game/game_data.hpp"
#include "ui/editor_commands.hpp"

void LevelsUi::draw(
    LevelsData &levels,
    const std::string &levelPath,
    EditorCommands &commands,
    bool levelHasUnsavedChanges)
{
    std::string playing = levelPath;
    std::optional<std::string> chosen;
    if (drawFileChooser("playing", playing, directoryOf(levelPath), ".json"))
        chosen = playing;

    bool switchPressed = false;
    bool cancelPressed = false;
    if (askedToSwitchTo && levelHasUnsavedChanges)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, UnsavedColour);
        ImGui::TextWrapped(
            "switching to %s discards unsaved changes to %s",
            levelName(*askedToSwitchTo).c_str(),
            levelName(levelPath).c_str());
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

    inspector::draw("first", levels.first);
}
void LevelsUi::revert(LevelsData &levels)
{
    revertTo(saveable, "levels", levels);
}

void LevelsUi::save(const LevelsData &levels)
{
    saveLevels(levels);
    saveable.saved("levels", asJson(levels));
}

std::optional<std::string> LevelsUi::cannotSaveBecause(const LevelsData &levels)
{
    return firstLevelGate.to(
        levels.first.path,
        [&]() -> std::optional<std::string>
        {
            if (!readLevelDataIfYouCan(levels.first.path))
                return "the first level \"" + levels.first.path + "\" cannot be read";

            return std::nullopt;
        });
}

bool LevelsUi::unsavedSince(const LevelsData &levels)
{
    return saveable.unsavedSince("levels", asJson(levels));
}

void LevelsUi::reloaded(LevelsData &current, const LevelsData &onDisk)
{
    askedToSwitchTo.reset();
    reload(saveable, "levels", current, onDisk);
}
