#include <cfloat>
#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <imgui.h>
#include "ui/renaming.hpp"
#include "ui/level_rewriting.hpp"
#include "ui/renames.hpp"
#include "ui/unsaved_colours.hpp"
#include "game/levels.hpp"
#include "game/level_data.hpp"

namespace
{
    void drawWrapped(const ImVec4 &colour, const std::string &line)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, colour);
        ImGui::TextWrapped("%s", line.c_str());
        ImGui::PopStyleColor();
    }

    bool drawNameField(std::string &name)
    {
        std::array<char, 256> buffer{};
        name.copy(buffer.data(), std::min(name.size(), buffer.size() - 1));

        ImGui::TextUnformatted("name");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        bool entered = ImGui::InputText(
            "##name", buffer.data(), buffer.size(), ImGuiInputTextFlags_EnterReturnsTrue);
        name = buffer.data();

        return entered;
    }
}

std::optional<std::string> whyNotARename(
    std::string_view what,
    const std::string &from,
    const std::string &to,
    bool taken)
{
    if (to.empty())
        return std::string(what) + " needs a name";

    if (to == from)
        return std::nullopt;

    if (taken)
        return "there is already " + std::string(what) + " called \"" + to + "\"";

    return std::nullopt;
}

void rememberRename(Renames &renames, const std::string &from, const std::string &to)
{
    std::string onDisk = from;
    for (const auto &[was, is] : renames)
        if (is == from)
            onDisk = was;

    if (onDisk == to)
        renames.erase(onDisk);
    else
        renames.insert_or_assign(onDisk, to);
}

std::optional<Renamed> Renaming::draw(
    std::string_view what,
    const std::string &selected,
    const NameTaken &taken)
{
    if (selected != lastSelected)
    {
        lastSelected = selected;
        typing = shownName(selected);
    }

    bool entered = drawNameField(typing);

    std::optional<std::string> why =
        whyNotARename(what, shownName(selected), typing, taken(typing));
    if (why)
        drawWrapped(CannotSaveColour, *why);
    else
        drawWhatTheLevelsNeed();

    if (!entered || why || typing == shownName(selected))
        return std::nullopt;

    Renamed renamed{shownName(selected), typing};
    rememberRename(renames, renamed.from, renamed.to);
    rePointed.clear();

    return renamed;
}

void Renaming::drawWhatTheLevelsNeed() const
{
    if (std::optional<std::string> cannot = cannotSaveBecause())
        drawWrapped(CannotSaveColour, *cannot);
    else if (std::string levels = whatTheLevelsNeed(); !levels.empty())
        drawWrapped(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), levels);
}

void Renaming::added(const std::string &name)
{
    neverSaved.insert(name);
}

bool Renaming::remove(const std::string &onDisk, const std::string &fallingBackTo)
{
    renames.erase(onDisk);
    rePointed.clear();

    if (neverSaved.erase(onDisk) > 0)
        return false;

    removals.insert_or_assign(onDisk, fallingBackTo);
    return true;
}

bool Renaming::gone(const std::string &onDisk) const
{
    return removals.contains(onDisk);
}

std::vector<std::string> Renaming::removed() const
{
    std::vector<std::string> names;
    for (const auto &[was, fallsBackTo] : removals)
        names.push_back(was);

    return names;
}

Renames Renaming::sinceSaved() const
{
    Renames pending = renames;
    for (const auto &[was, fallsBackTo] : removals)
        if (!somethingIsBecoming(was))
            pending.insert({was, fallsBackTo});

    return pending;
}

std::string nameAfterRenames(const Renames &renames, const std::string &name)
{
    auto renamed = renames.find(name);
    return renamed == renames.end() ? name : renamed->second;
}

std::string levelsInAList(const std::vector<std::string> &levelPaths)
{
    std::string listed;
    for (std::size_t at = 0; at < levelPaths.size(); ++at)
    {
        if (at > 0)
            listed += at + 1 == levelPaths.size() ? " and " : ", ";

        listed += levelName(levelPaths[at]);
    }

    return listed;
}

std::string Renaming::shownName(const std::string &onDisk) const
{
    return nameAfterRenames(renames, onDisk);
}

std::optional<std::string> Renaming::cannotSaveBecause() const
{
    if (unreadable.empty())
        return std::nullopt;

    return levelsInAList(unreadable) + " cannot be read";
}

std::string Renaming::whatTheLevelsNeed() const
{
    if (!willRePoint.empty())
        return levelsInAList(willRePoint) + " will be re-pointed.";

    if (!rePointed.empty())
        return levelsInAList(rePointed) + " re-pointed.";

    return {};
}

bool Renaming::somethingIsBecoming(const std::string &name) const
{
    for (const auto &[was, is] : renames)
        if (is == name)
            return true;

    return false;
}

void Renaming::applied(const std::vector<std::string> &levels)
{
    renames.clear();
    removals.clear();
    neverSaved.clear();
    willRePoint.clear();
    unreadable.clear();
    rePointed = levels;
}

void Renaming::willReach(const std::vector<std::string> &levels)
{
    willRePoint = levels;
}

void Renaming::cannotReach(const std::vector<std::string> &levels)
{
    unreadable = levels;
}

void Renaming::forget()
{
    typing.clear();
    lastSelected.clear();
    renames.clear();
    removals.clear();
    neverSaved.clear();
    rePointed.clear();
    willRePoint.clear();
    unreadable.clear();
}

void lookAheadAtLevels(
    Renaming &renaming,
    const std::string &directory,
    const std::function<bool(LevelData &, const Renames &)> &rename)
{
    const Renames renames = renaming.sinceSaved();
    rewriting::Reach reach = rewriting::whatItWouldReach(
        directory, [&](LevelData &levelData) { return rename(levelData, renames); });

    renaming.willReach(reach.levels);
    renaming.cannotReach(reach.unreadable);
}

bool writeRenamesIntoLevels(
    Renaming &renaming,
    const std::string &directory,
    const std::function<bool(LevelData &, const Renames &)> &rename)
{
    const Renames renames = renaming.sinceSaved();
    if (renames.empty())
        return true;

    rewriting::Reach reach = rewriting::theLevels(
        directory, [&](LevelData &levelData) { return rename(levelData, renames); });
    if (!reach.unreadable.empty())
    {
        renaming.cannotReach(reach.unreadable);
        return false;
    }

    renaming.applied(reach.levels);
    return true;
}
