#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <imgui.h>
#include "ui/levels_ui.hpp"
#include "ui/saved_in_scope.hpp"
#include "ui/data_inspector.hpp"
#include "game/empty_level.hpp"
#include "game/level_data_file.hpp"
#include "ui/file_chooser.hpp"
#include "ui/level_rewriting.hpp"
#include "ui/renames.hpp"
#include "ui/renaming.hpp"
#include "ui/switching_level.hpp"
#include "ui/unsaved_colours.hpp"
#include "ui/saveable.hpp"
#include "game/levels.hpp"
#include "game/levels_data.hpp"
#include "ui/editor_commands.hpp"

namespace
{
    bool nextLevelIn(LevelData &levelData, const Renames &renames)
    {
        return rewriting::nextLevelIn(levelData, renames);
    }
}

LevelsUi::LevelsUi(std::string levelsDirectory, WriteLevels writeLevels)
    : levelsDirectory(std::move(levelsDirectory)), writeLevels(std::move(writeLevels))
{
}

std::vector<std::string> LevelsUi::offered() const
{
    std::vector<std::string> offers;
    for (const std::string &path : levelPathsIn(levelsDirectory))
        if (!renaming.gone(path))
            offers.push_back(path);

    for (const std::string &path : made)
        if (std::find(offers.begin(), offers.end(), path) == offers.end())
            offers.push_back(path);

    return offers;
}

std::optional<std::string> LevelsUi::firstRemainingAfter(const std::string &leaving) const
{
    for (const std::string &path : offered())
        if (path != leaving)
            return path;

    return std::nullopt;
}

std::string LevelsUi::add(const LevelData &playing, int tileSize, EditorCommands &commands)
{
    std::string path = aLevelPathNobodyHasTaken(levelsDirectory, made);
    made.push_back(path);
    renaming.added(path);
    commands.onPlayLevel(path, anEmptyLevelLike(playing, tileSize));

    return path;
}

void LevelsUi::remove(const std::string &levelPath, EditorCommands &commands)
{
    std::optional<std::string> fallingBackTo = firstRemainingAfter(levelPath);
    if (renaming.remove(levelPath, fallingBackTo))
        lookAheadAtLevels(renaming, levelsDirectory, nextLevelIn);

    std::erase(made, levelPath);
    firstLevelGate.forgets();

    if (fallingBackTo)
        commands.onLoadLevel(*fallingBackTo);
}

LevelsAsked LevelsUi::draw(
    LevelsData &levels,
    const LevelData &playing,
    const std::string &levelPath,
    int tileSize,
    EditorCommands &commands,
    bool levelHasUnsavedChanges)
{
    LevelsAsked asked;
    ImGui::PushID("levels");

    std::string chosenPath = levelPath;
    std::optional<std::string> chosen;
    if (drawFileChooser("playing", chosenPath, levelsDirectory, offered()))
        chosen = chosenPath;

    if (ImGui::Button("add"))
        asked.made = add(playing, tileSize, commands);

    ImGui::SameLine();
    ImGui::BeginDisabled(levelPath.empty());
    if (ImGui::Button("remove"))
        remove(levelPath, commands);

    ImGui::EndDisabled();
    renaming.drawWhatTheLevelsNeed();

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

    SavedInScope was(asItWasSaved<LevelsData>(saveable.lastSeen("levels")));
    inspector::draw("first", levels.first);

    ImGui::PopID();

    return asked;
}

std::string LevelsUi::revert(LevelsData &levels, const std::string &levelPath)
{
    bool playingHasNoFile = std::find(made.begin(), made.end(), levelPath) != made.end();
    std::optional<std::string> instead = firstRemainingAfter(levelPath);

    made.clear();
    revertTo(saveable, "levels", levels, renaming);
    firstLevelGate.forgets();

    if (playingHasNoFile && instead)
        return *instead;

    return levelPath;
}

bool LevelsUi::save(LevelsData &levels, LevelData &playing)
{
    Renames pending = renaming.sinceSaved();
    std::vector<std::string> removed = renaming.removed();

    if (!writeRenamesIntoLevels(renaming, levelsDirectory, nextLevelIn))
        return false;

    for (const std::string &levelPath : removed)
        removeLevelData(levelPath);

    levels.first.path = nameAfterRenames(pending, levels.first.path);
    renaming.applied();
    firstLevelGate.forgets();
    made.clear();

    writeLevels(levels);
    saveable.saved("levels", asJson(levels));

    return rewriting::nextLevelIn(playing, pending);
}

std::optional<std::string> LevelsUi::cannotSaveBecause(const LevelsData &levels)
{
    if (std::optional<std::string> renames = renaming.cannotSaveBecause())
        return renames;

    const std::string &first = levels.first.path;
    if (renaming.gone(first) && nameAfterRenames(renaming.sinceSaved(), first) == first)
        return "the first level \"" + levelName(first) + "\" is being removed";

    return firstLevelGate.to(
        first,
        [&]() -> std::optional<std::string>
        {
            if (!readLevelDataIfYouCan(first))
                return "the first level \"" + first + "\" cannot be read";

            return std::nullopt;
        });
}

bool LevelsUi::unsavedSince(const LevelsData &levels)
{
    return saveable.unsavedSince("levels", asJson(levels)) || renaming.pending();
}

void LevelsUi::reloaded(LevelsData &current, const LevelsData &onDisk)
{
    askedToSwitchTo.reset();
    reload(saveable, "levels", current, onDisk);
}
