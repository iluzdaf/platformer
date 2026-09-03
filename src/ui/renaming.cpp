#include <cfloat>
#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/renaming.hpp"
#include "game/renames.hpp"
#include "ui/unsaved_colours.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "assets/asset_paths.hpp"

namespace
{
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
        typing = selected;
    }

    bool entered = drawNameField(typing);

    std::optional<std::string> why = whyNotARename(what, selected, typing, taken(typing));
    if (why)
        ImGui::TextColored(CannotSaveColour, "%s", why->c_str());
    else if (rewritten)
        ImGui::TextDisabled("%d level%s re-pointed", *rewritten, *rewritten == 1 ? "" : "s");

    if (!entered || why || typing == selected)
        return std::nullopt;

    Renamed renamed{selected, typing};
    rememberRename(renames, renamed.from, renamed.to);
    rewritten.reset();
    lastSelected = renamed.to;

    return renamed;
}

const Renames &Renaming::sinceSaved() const
{
    return renames;
}

void Renaming::applied(int levelsRewritten)
{
    renames.clear();
    rewritten = levelsRewritten > 0 ? std::optional<int>(levelsRewritten) : std::nullopt;
}

void Renaming::forget()
{
    typing.clear();
    lastSelected.clear();
    renames.clear();
    rewritten.reset();
}

void writeRenamesIntoLevels(
    Renaming &renaming,
    const std::function<bool(LevelData &, const Renames &)> &rename)
{
    const Renames &renames = renaming.sinceSaved();
    if (renames.empty())
        return;

    int rewritten = renameInLevels(
        std::string(assets::Levels),
        [&](LevelData &levelData) { return rename(levelData, renames); });

    renaming.applied(rewritten);
}
